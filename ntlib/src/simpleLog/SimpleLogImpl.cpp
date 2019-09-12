#include "SimpleLogImpl.h"

#include <fstream>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <climits>
#include <assert.h>
#include <sstream>
#include <algorithm>

#ifdef _MSC_VER
#include <io.h>
#define snprintf(_DstBuf, _SizeInBytes, format,...) _snprintf_s(_DstBuf, _SizeInBytes, _TRUNCATE, format,##__VA_ARGS__)
#define localtime_r(time_t, tm) localtime_s(tm, time_t)
#else
#include <unistd.h>
#include <sys/stat.h>
#endif
namespace nt
{

namespace simpleLog
{

#define LOG_STD_OUT "stdout"

namespace detail
{
void GetCurTime(struct tm *t)
{
    time_t timep;
    time(&timep);
    localtime_r(&timep, t);
}

std::string GetSystemDayTime2()
{
    char timebuf[64] = {0};

    struct tm t;
    GetCurTime(&t);
    snprintf(timebuf, sizeof(timebuf), "%4d-%02d-%02d %02d:%02d:%02d",
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    return std::string(timebuf);
}

std::string GetSystemDay(const struct tm &t)
{
    char Time[30] = {0};
    snprintf(Time, sizeof(Time) - 1, "%4d-%02d-%02d", 1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday);
    std::string SystemTime(Time);
    return SystemTime;
}

std::string GetSystemDayHour(const struct tm &t)
{
    char timebuf[64] = {0};
    snprintf(timebuf, sizeof(timebuf), "%4d-%02d-%02d_%02d",
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour);
    return std::string(timebuf);
}

bool TimeOverDay(const struct tm &ptime)
{
    struct tm now;
    time_t t = time(NULL);
    localtime_r(&t, &now);

    return ((now.tm_mday != ptime.tm_mday) ||
            (now.tm_mon != ptime.tm_mon) ||
            (now.tm_year != ptime.tm_year));
}

bool TimeOverHour(const struct tm &ptime)
{
    struct tm now;
    time_t t = time(NULL);
    localtime_r(&t, &now);

    return ((now.tm_hour != ptime.tm_hour) ||
            (now.tm_mday != ptime.tm_mday) ||
            (now.tm_mon != ptime.tm_mon) ||
            (now.tm_year != ptime.tm_year));
}

static bool IsConsole(const char *file)
{
    return strcmp(LOG_STD_OUT, file) == 0;
}

static int LoadFile(const char *file, Parser *handle)
{
    FILE *fp = fopen(file, "r");
    if (NULL != fp)
    {
        char line[256] = { 0 };
        for (int i = 1; NULL != fgets(line, sizeof(line), fp); i++)
        {
            handle->parser(line);
        }

        return 0;
    }
    return -1;
}

static std::vector<std::string> split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

bool IsSpace (int ch)
{
    return !std::isspace(ch);
}

// trim from start (in place)
static inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), IsSpace));
}

// trim from end (in place)
static inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), IsSpace).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s)
{
    ltrim(s);
    rtrim(s);
}

}; // namespace detail

using namespace detail;
std::string simpleLogTool::getNewFileName(const std::string &oldname, int type, const struct tm &t)
{
    std::string tryname (oldname);

    std::string dateSuffix = ".";
    if(type == ROLL_PER_HOUR)
    {
        dateSuffix += GetSystemDayHour(t);
    }
    else
    {
        dateSuffix += GetSystemDay(t);
    }
    tryname += dateSuffix;
    std::string newname(tryname);
    char numSuffix[64 + 2] = {0};
    for(int i = 1; isExist(newname) && i < INT_MAX; i++)
    {
        newname = tryname;
        memset(numSuffix, 0, sizeof(numSuffix));
        snprintf(numSuffix, sizeof(numSuffix), ".%d", i);
        newname += numSuffix;
    }
    return newname;
}

bool simpleLogTool::isExist(const std::string &file)
{
    std::ifstream ifs(file.c_str());
    return ifs.good();
}
bool simpleLogTool::isFileEmpty(const std::string &file)
{
    std::ifstream ifs(file.c_str());
    return ifs.peek() == std::ifstream::traits_type::eof();
}

void simpleLogTool::newfile(const std::string &name)
{
    std::ofstream ofs(name.c_str());
    if (!ofs)
    {
        printf("[simpleLog::Log] Cannot open the output file: %s\n", name.c_str());
    }
    ofs.close();
}

/*
	enum
	{
		LOG_LELEVE_FATAL = 0,
		LOG_LELEVE_ERROR = 1,
		LOG_LELEVE_WARNING = 2,
		LOG_LELEVE_INFO = 3,
		LOG_LELEVE_DEBUG = 4,
		TOTLE_LOG_LELEVE_SIZE = 5
	};

	enum ConstructorMode
	{
		SET_NAME = 0,
		READ_CONFIG = 1  // Not implemented
	};

*/


SimpleLogImpl::SimpleLogImpl()
{
    m_roll = ROLL_DAILY;
    m_logLevel = 3;
    m_multithread = 0;

    m_fp = NULL;

    m_logfilename = LOG_STD_OUT;
}
SimpleLogImpl::~SimpleLogImpl()
{
    if(m_fp != NULL && m_fp != stdout)
    {
        fclose(m_fp);
    }
}

int SimpleLogImpl::start()
{
    if(m_logArgs.name.empty())
    {
        printf("[simpleLog::Log] log filename is empty\n");
        assert(false);
        return 1;
    }

    int ret = 2;

    switch(m_logArgs.mode)
    {
    case 0:
        m_logfilename = m_logArgs.name;
        ret = 0;
        break;
    case 1:
        ret = readConfig(m_logArgs.name.c_str());
        break;
    default:
        assert(false);
        break;
    }
    if(ret != 0)
    {
        return ret;
    }
    setRoll(m_roll);

    struct stat statBuf;
    time_t t;

    ret = ::stat(m_logfilename.c_str(), &statBuf);
    if (ret < 0)
    {
        t = time(NULL);
    }
    else
    {
        t = statBuf.st_mtime;
    }

    localtime_r(&t, &m_lastTime);

    ret = openFile(m_logfilename.c_str());

    return ret;
}

void SimpleLogImpl::setRoll(int roll)
{
    if(IsConsole(m_logfilename.c_str()))
    {
        m_roll = 0;
    }
    else
    {
        m_roll = roll;
    }
}

int SimpleLogImpl::getRoll() const
{
    return m_roll;
}

void SimpleLogImpl::setLogLevel(int level)
{
    m_logLevel = level;
}

int SimpleLogImpl::getLogLevel() const
{
    return m_logLevel;
}

void SimpleLogImpl::setMultithread(int m)
{
    m_multithread = m;
}

int SimpleLogImpl::getMultithread() const
{
    return m_multithread;
}

const std::string &SimpleLogImpl::getLogFileName() const
{
    return m_logfilename;
}

int SimpleLogImpl::checkLogLevel(int log_level)
{
    if(getLogLevel() < log_level)
    {
        return -1;
    }
    return 0;
}

int SimpleLogImpl::openFile(const char *file)
{
    int ret = -1;

    if (m_fp != NULL && m_fp != stdout)
    {
        fclose(m_fp);
        m_fp = NULL;
    }

    if(IsConsole(file))
    {
        m_fp = stdout;
        setbuf(m_fp, NULL);
        ret = 0;
    }
    else
    {
        m_fp = fopen(file, "a");
        if(m_fp == NULL)
        {
            printf("[simpleLog::openFile] Cannot open the output file: %s\n", file);

            ret = -1;
        }
        setbuf(m_fp, NULL);
        ret = 0;
    }

    return ret;
}

void SimpleLogImpl::logImpl(FILE *stream, const char *fmt, va_list argList)
{
    struct tm t;
    GetCurTime(&t);
    fprintf(stream, "%4d-%02d-%02d %02d:%02d:%02d ",
            t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

    vfprintf (stream, fmt, argList);

    fprintf(stream, "\n");
}

void SimpleLogImpl::print_log(int log_level, const char *fmt, va_list argList)
{
    if(checkLogLevel(log_level) != 0)
    {
        return;
    }

    if(getMultithread())
    {
        m_mutex.lock();
    }

    checkbackup();

    logImpl(m_fp, fmt, argList);

    if(getMultithread())
    {
        m_mutex.unlock();
    }
}

int SimpleLogImpl::checkbackup()
{
    if(timeOver(m_roll))
    {
        dobackup();

        time_t t = time(NULL);
        localtime_r(&t, &m_lastTime);
    }

    return 0;
}

int SimpleLogImpl::dobackup()
{
    const std::string &oldfile = getLogFileName();
    try
    {
        ScopedLock<ThreadMutex> lock(m_mutex);
        if(simpleLogTool::isExist(oldfile))
        {
            rename(oldfile.c_str(), simpleLogTool::getNewFileName(oldfile, m_roll, m_lastTime).c_str());
        }
        openFile(getLogFileName().c_str());
    }
    catch(std::exception &ex)
    {
        return -1;
        printf("[simpleLog::Log] [dobackup] %s\n", ex.what());
    }

    return 0;
}
bool SimpleLogImpl::timeOver(int type)
{
    if (ROLL_DAILY == type)
    {
        return TimeOverDay(m_lastTime);
    }
    else if (ROLL_PER_HOUR == type)
    {
        return TimeOverHour(m_lastTime);
    }
    return false;
}

int SimpleLogImpl::parser(const char *line)
{
#define LOG_CONFIG_COMMENT '#'

    std::string str(line);
    trim(str);
    if(str.empty() || str[0] == LOG_CONFIG_COMMENT)
    {
        return 1;
    }
    std::vector<std::string> vecStr = split(str, '=');
    if(vecStr.size() < 2)
    {
        return 2;
    }
    trim(vecStr[0]);
    trim(vecStr[1]);

    if(vecStr[0].empty() || vecStr[1].empty())
    {
        return 3;
    }

    m_prop.push_back(std::make_pair(vecStr[0], vecStr[1]));
    return 0;
}

struct NameToNum
{
    const char *name;
    int num;
};

int SimpleLogImpl::loadProp()
{
    using namespace std;
#define SP "log4cplus."
#define SP2 "log4cplus::"
    string nick;
    for(prop_t::const_iterator itr = m_prop.begin(); itr != m_prop.end(); ++itr)
    {
        const string &key = itr->first;
        const string &value = itr->second;
        if(SP"rootLogger" == key)
        {
            std::vector<std::string> vecStr = split(value, ',');
            trim(vecStr[0]);
            string level = vecStr[0];
            NameToNum nameToNums[] =
            {
                {"FATAL", 0},
                {"0", 0},
                {"ERROR", 1},
                {"1", 1},
                {"WARN", 2},
                {"2", 2},
                {"INFO", 3},
                {"3", 3},
                {"DEBUG", 4},
                {"4", 4},
                {"ALL", 5},
                {"5", 5},
                {NULL, 0}
            };

            for( int i = 0; ; ++i )
            {
                NameToNum *iter = &(nameToNums[i]);
                if( NULL == iter->name ) break;

                if( 0 == strcasecmp( level.c_str(), iter->name ) )
                {
                    m_logLevel = iter->num;
                }
            }
            if(vecStr.size() >= 2)
            {
                trim(vecStr[1]);
                nick = string(".") + vecStr[1];
            }
        }
        else if( (std::string(SP"appender") + nick + ".Schedule") == key)
        {
            NameToNum nameToNums[] =
            {
                {"DAILY", ROLL_DAILY},
                {"1", ROLL_DAILY},
                {"HOURLY", ROLL_PER_HOUR},
                {"2", ROLL_PER_HOUR},
                {NULL, 0}
            };

            for( int i = 0; ; ++i )
            {
                NameToNum *iter = &(nameToNums[i]);
                if( NULL == iter->name ) break;

                if( 0 == strcasecmp( value.c_str(), iter->name ) )
                {
                    m_roll = iter->num;
                }
            }
        }
        else if( (std::string(SP"appender") + nick + ".File") == key)
        {
            m_logfilename = value;
        }
        else if( (std::string(SP"appender") + nick + ".Multithread") == key)
        {
            NameToNum nameToNums[] =
            {
                {"yes", 1},
                {"no", 0},
                {"y", 1},
                {"n", 0},
                {NULL, 0}
            };

            for( int i = 0; ; ++i )
            {
                NameToNum *iter = &(nameToNums[i]);
                if( NULL == iter->name ) break;

                if( 0 == strcasecmp( value.c_str(), iter->name ) )
                {
                    m_multithread = iter->num;
                }
            }
        }
    }
    return 0;
}

int SimpleLogImpl::readConfig(const char *configFile)
{
    m_prop.clear();
    int ret = 0;
    ret = LoadFile(configFile, this);
    if(ret != 0)
    {
        return -1;
    }
    ret = loadProp();

    if(ret != 0)
    {
        return -2;
    }
    return 0;
}

}; //namespace simpleLog

} //namespace nt
