#pragma once

#include <string>
#include <sstream>
#include <map>
#include <time.h>
namespace nt
{
class HttpSession;
#define SESSION_SIZE 26
#define MAX_SESSION_BUF_SIZE 8192

std::string CreateSessionId();
void SessionSetSessionId(HttpSession *session, const std::string &id);
void SessionCreateSessionId(HttpSession *session);
std::string SessionToString(HttpSession *session);
int SessionFromString(HttpSession *session, const std::string &src);

bool SessionIdValid(const std::string &sessionId);

class HttpSession
{
public:
    HttpSession();
    ~HttpSession();

    const std::string *GetAttribute(const std::string &key);

    template<typename T>
    int SetAttribute(const std::string &key, const T &value)
    {
        std::stringstream ss;
        ss << value;

        return DoSetAttribute(key, ss.str());
    }

    void RemoveAttribute(const std::string &key);
    time_t GetCreationTime();
    long GetLastAccessedTime();

    void Invalidate();

    const std::string &GetSessionId() const;
private:
    friend void SessionSetSessionId(HttpSession *session, const std::string &id);
    friend void SessionCreateSessionId(HttpSession *session);
    friend std::string SessionToString(HttpSession *session);
    friend int SessionFromString(HttpSession *session, const std::string &src);

    int DoSetAttribute(const std::string &key, const std::string &value);

    std::string m_sessionId;
    time_t m_creationTime;
    time_t m_lastAccessedTime;
    int m_maxInactiveInterval;

    std::map<std::string, std::string> m_attr;
};



};

