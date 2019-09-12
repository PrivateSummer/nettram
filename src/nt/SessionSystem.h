#pragma once

#include <string>
#include <sstream>
#include <map>

namespace nt
{
#define SESSION_DEFAULT_INACTIVE_INTERVAL 600

class HttpSession;

class SessionSystem
{
public:
	static SessionSystem *Instance();

	HttpSession* Get(const std::string &sessionId);
    HttpSession* MutableGet(const std::string &sessionId);

	int Set(const std::string &sessionId, HttpSession *session);

	void Remove(const std::string &sessionId);

    int GetMaxInactiveInterval();
	void SetMaxInactiveInterval(int interval);
private:
    SessionSystem();
	SessionSystem & operator=(SessionSystem&);
	SessionSystem(SessionSystem&);
	
	int SetFile(int fd, HttpSession *session);
	
	HttpSession* GetFile(const char *filename);
	
	void CheckTimeClearFiles();
	int ClearFiles();
	int CheckTimeClearFile(const char *file);
	
private:
    int m_maxInactiveInterval;
	int m_last_clear_time;
};


};

