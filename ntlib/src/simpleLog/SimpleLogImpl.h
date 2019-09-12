#include "SimpleMutex.h"
#include <string>
#include <vector>
#include <stdio.h>
namespace nt
{

namespace simpleLog
{

class simpleLogTool
{
public:
	static std::string getNewFileName(const std::string & oldname, int type, const struct tm& t);
	static bool isExist(const std::string &file);
	static bool isFileEmpty(const std::string &file);
	static void newfile(const std::string &name);
};

enum ROLL_TYPE
{
	ROLL_NONE = 0,     //不滚动
	ROLL_DAILY = 1,    //按日滚动
	ROLL_PER_HOUR = 2, //按小时滚动
};

class Parser
{
public:
	virtual ~Parser(){};
	virtual int parser(const char *line) = 0;
};

class SimpleLogImpl : public Parser
{
public:

    SimpleLogImpl();
	~SimpleLogImpl();
	
	int start();
	
	void print_log(int log_level, const char *fmt, va_list argList);
	
	int checkLogLevel(int log_level);
	
	const std::string & getLogFileName() const;
	
public:	
	void setRoll(int roll);
	int getRoll() const;
	
	/* 日志级别. 默认 LOG_LELEVE_INFO (3) */
	void setLogLevel(int level);
	int getLogLevel() const;
	
	/* 多线程支持. 1支持, 0不支持. 默认 0 */
	void setMultithread(int m);
	int getMultithread() const;

	struct LogArgs
	{
		std::string name;
		int mode;
	};
	
	LogArgs m_logArgs;
	
private:
    SimpleLogImpl(const SimpleLogImpl &);
	SimpleLogImpl & operator=(const SimpleLogImpl &);	
	
private:
    virtual int parser(const char *line);
	
	void logImpl(FILE * stream, const char *fmt, va_list argList);
	int openFile(const char *file);
	
	int checkbackup();
	int dobackup();
	
	bool timeOver(int type);

	int readConfig(const char *configFile);
	
	int loadProp();
	
	std::string m_logfilename;	
	
	
	int m_roll;
	int m_logLevel;
	int m_multithread;
	ThreadMutex m_mutex;
	
	FILE *m_fp;
	struct tm m_lastTime;
	typedef std::vector<std::pair<std::string, std::string> > prop_t;
	prop_t m_prop;
};

}//namespace simpleLog

}//namespace nt
