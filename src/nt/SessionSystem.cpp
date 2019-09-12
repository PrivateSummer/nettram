#include "SessionSystem.h"
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "log.h"
#include "HttpSession.h"
#include "CTime.h"

namespace nt
{
#define SESSION_DIR "session"
#define SESSION_FILE SESSION_DIR"/session_"

static void create_session_file_name(const std::string &sessionId, char *buf, int buf_len)
{
	int n = snprintf(buf, buf_len, "%s%s", SESSION_FILE, sessionId.c_str());
	buf[n] = '\0';
}
SessionSystem::SessionSystem() 
{
	m_maxInactiveInterval = SESSION_DEFAULT_INACTIVE_INTERVAL;
	ClearFiles();
	m_last_clear_time = time(NULL);
}

SessionSystem *SessionSystem::Instance()
{
	static SessionSystem s_session;
	return &s_session;
}
HttpSession* SessionSystem::Get(const std::string &sessionId)
{
	if(sessionId.empty())
	{
		return NULL;
	}
	
	char filename[256];
	create_session_file_name(sessionId, filename, sizeof(filename) - 1);
	
	HttpSession *session = NULL;
	
	int ret = CheckTimeClearFile(filename);
	if(ret > 0)
	{
		session = GetFile(filename);
	}
	
	debug_log("strSession: %s", SessionToString(session).c_str());

	return session;
}

HttpSession* SessionSystem::MutableGet(const std::string &sessionId)
{
	HttpSession *session = NULL;
	std::string id = sessionId;
	if(id.empty())
	{
		id = CreateSessionId();
	}
	char filename[256];
	create_session_file_name(id, filename, sizeof(filename) - 1);
	
	if((access(filename,F_OK)) != -1) //文件存在
    {
		CheckTimeClearFile(filename);
		
		if((session = GetFile(filename)) != NULL)
		{
			return session;
		} 
    }
	
	int fd = open(filename, O_RDWR|O_CREAT, S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	if(fd < 0)
	{
		info_log("open %s ret:%d, errno:%d, strerror:%s", filename, fd, errno, strerror(errno));
		
		return NULL;
	}
	
	session = new HttpSession();
	SessionSetSessionId(session, id);
	
	int ret = SetFile(fd, session);
	close(fd);
	if(ret != 0)
	{
		delete session;
		session = NULL;
		return NULL;
	}
	
	return session;
}

int SessionSystem::Set(const std::string &sessionId, HttpSession *session)
{
	if(sessionId.empty())
	{
		return 1;
	}

    char filename[256];
	create_session_file_name(sessionId, filename, sizeof(filename) - 1);
	
	int fd = open(filename, O_RDWR|O_CREAT, S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	if(fd < 0)
	{
		info_log("open %s ret:%d, errno:%d, strerror:%s", filename, fd, errno, strerror(errno));
		
		return -1;
	}
	int ret = SetFile(fd, session);
	close(fd);
	return ret;
}
void SessionSystem::Remove(const std::string &sessionId)
{
	if(SessionIdValid(sessionId))
	{
		char filename[256];
		create_session_file_name(sessionId, filename, sizeof(filename) - 1);
		remove(filename);
	}
}

int SessionSystem::SetFile(int fd, HttpSession *session)
{
	std::string strSession = SessionToString(session);
	
	debug_log("strSession: %s", strSession.c_str());
	int size = write( fd, strSession.c_str(), strSession.size());
	if (size < 0)
	{
        info_log("write ret:%d, errno:%d, strerror:%s", size, errno, strerror(errno));
		return -1;
    }
	return 0;
}
HttpSession* SessionSystem::GetFile(const char *filename)
{
	HttpSession *session = NULL;

	int fd = open(filename, O_RDWR, S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	if(fd < 0)
	{
		info_log("open %s ret:%d, errno:%d, strerror:%s", filename, fd, errno, strerror(errno));
		
		return NULL;
	}
	lseek(fd, 0, SEEK_SET );
	char buf[MAX_SESSION_BUF_SIZE];
	int size = read( fd, buf, MAX_SESSION_BUF_SIZE - 1);
	close(fd);
	if(size < 0) 
	{
		info_log("read %s ret:%d, errno:%d, strerror:%s", filename, size, errno, strerror(errno));
		return NULL;
	}
	buf[size] = '\0';
	
	session = new HttpSession();
	int ret = SessionFromString(session, buf);
	if(ret != 0)
	{
		delete session;
		session = NULL;
		err_log("SessionFromString failed:%d", ret);
	}

	return session;
}

int SessionSystem::ClearFiles()
{
	CTime ctime;
	struct dirent* ptr;

	const char *bashDir = SESSION_DIR;
	DIR* dirp = opendir(bashDir);
	if (dirp == NULL)
	{
		err_log("opendir %s, errno:%d, strerror:%s", bashDir, errno, strerror(errno));
		return -1;
	}
	
	char file[1024];
	
	while ((ptr = readdir(dirp)) != NULL)
	{
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)    ///current dir OR parrent dir
		{
			continue;
		}

		int n = snprintf(file, sizeof(file) - 1, "%s/%s", bashDir, ptr->d_name);
		file[n] = '\0';

        CheckTimeClearFile(file);
	}
	closedir(dirp);
	debug_log("SessionSystem::ClearFiles in %d ms", ctime.getFuncRunTime());
	return 0;
}

int SessionSystem::CheckTimeClearFile(const char *file)
{
	struct stat statBuf;
	int ret = stat(file, &statBuf);
	if (ret != 0)
	{
		info_log("stat %s, errno:%d, strerror:%s", file, errno, strerror(errno));
		return -1;
	}
	
	time_t timep = statBuf.st_mtime;

	time_t now = time(NULL);
	if(now > timep + GetMaxInactiveInterval())
	{
		remove(file);
		printf("remove session file:%s\n", file);
		
		return 0;
	}
	
	return 1;
}

void SessionSystem::CheckTimeClearFiles()
{
	time_t now = time(NULL);
	if(now > m_last_clear_time + SESSION_DEFAULT_INACTIVE_INTERVAL)
	{
		ClearFiles();
		m_last_clear_time = now;
	}
}

int SessionSystem::GetMaxInactiveInterval()
{
	return m_maxInactiveInterval;
}
void SessionSystem::SetMaxInactiveInterval(int interval)
{
	m_maxInactiveInterval = interval;
}

}// namespace nt
