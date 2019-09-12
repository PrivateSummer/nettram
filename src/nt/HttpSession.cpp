#include "HttpSession.h"
#include <errno.h>
#include <stdlib.h>
#include <vector>
#include "log.h"
#include "SessionSystem.h"
#include "str.h"

namespace nt
{

static void gen_random(char *s, const int len)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i)
    {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}

HttpSession::HttpSession()
{
    m_creationTime = time(NULL);
    m_lastAccessedTime = m_creationTime;
    m_maxInactiveInterval = 5 * 60;
}

HttpSession::~HttpSession()
{}

const std::string *HttpSession::GetAttribute(const std::string &key)
{
    const std::string *attr = NULL;
    m_lastAccessedTime = time(NULL);

    std::map<std::string, std::string>::const_iterator itr = m_attr.find(key);
    if(itr != m_attr.end())
    {
        attr = &itr->second;
    }

    SessionSystem::Instance()->Set(m_sessionId, this);

    return attr;
}

int HttpSession::DoSetAttribute(const std::string &key, const std::string &value)
{
    m_lastAccessedTime = time(NULL);
    m_attr[key] = value;

    SessionSystem::Instance()->Set(m_sessionId, this);
}

void HttpSession::RemoveAttribute(const std::string &key)
{
    m_lastAccessedTime = time(NULL);
    m_attr.erase(key);

    SessionSystem::Instance()->Set(m_sessionId, this);
}


time_t HttpSession::GetCreationTime()
{
    return m_creationTime;
}
long HttpSession::GetLastAccessedTime()
{
    return m_lastAccessedTime;
}

void HttpSession::Invalidate()
{
    m_attr.clear();
}

const std::string &HttpSession::GetSessionId() const
{
    return m_sessionId;
}

void SessionSetSessionId(HttpSession *session, const std::string &id)
{
    session->m_sessionId = id;
}

void SessionCreateSessionId(HttpSession *session)
{
    SessionSetSessionId(session, CreateSessionId());
}

std::string CreateSessionId()
{
    char buf[SESSION_SIZE + 1];
    gen_random(buf, SESSION_SIZE);

    return buf;
}

std::string SessionToString(HttpSession *session)
{
    if(session == NULL)
    {
        return "";
    }
    char buf[MAX_SESSION_BUF_SIZE] = {0};
    int n = 0;
    n += snprintf(buf + n, sizeof(buf) - n - 1, "Id>%s", session->GetSessionId().c_str());
    n += snprintf(buf + n, sizeof(buf) - n - 1, "'CT>%u", (unsigned int)session->GetCreationTime());
    n += snprintf(buf + n, sizeof(buf) - n - 1, "'LAT>%u", (unsigned int)session->GetLastAccessedTime());

    n += snprintf(buf + n, sizeof(buf) - n - 1, "'Attr>");
    std::map<std::string, std::string>::iterator itr = session->m_attr.begin();

    for( ; itr != session->m_attr.end(); ++itr)
    {
        std::string key = itr->first;
        std::string value = itr->second;
        replaceAll(key, "'", "&apos;");
        replaceAll(key, ">", "&gt;");
        replaceAll(value, "'", "&apos;");
        replaceAll(value, ">", "&gt;");
        if(itr != session->m_attr.begin())
        {
            n += snprintf(buf + n, sizeof(buf) - n - 1, ":");
        }
        n += snprintf(buf + n, sizeof(buf) - n - 1, "%s=%s", key.c_str(), value.c_str());
    }

    return buf;
}

int SessionFromString(HttpSession *session, const std::string &src)
{
    std::vector<std::string> kv = split(src, '\'');
    if(kv.empty())
    {
        err_log("parse session failed: src empty");
        return 1;
    }
    for(size_t i = 0; i < kv.size(); i++)
    {
        std::vector<std::string> v = split(kv[i], '>');
        if(v.size() == 2)
        {
            std::string &value = v[1];
            if("Id" == v[0])
            {
                session->m_sessionId = value;
            }
            else if("CT" == v[0])
            {
                session->m_creationTime = strtoul(value.c_str(), NULL, 10);
            }
            else if("LAT" == v[0])
            {
                session->m_lastAccessedTime = strtoul(value.c_str(), NULL, 10);
            }
            else if("Attr" == v[0])
            {
                std::vector<std::string> attr_kv = split(value, ':');
                for(size_t i = 0; i < attr_kv.size(); i++)
                {
                    std::vector<std::string> attr_v = split(attr_kv[i], '=');
                    if(attr_v.size() == 2)
                    {
                        session->m_attr[attr_v[0]] = attr_v[1];
                    }
                    else
                    {
                        err_log("parse session failed: expect =");
                        return -1;
                    }
                }
            }
        }

    }

    return 0;
}

bool SessionIdValid(const std::string &sessionId)
{
    if(sessionId.size() != SESSION_SIZE)
    {
        return false;
    }
    for(size_t i = 0; i < sessionId.size(); i++)
    {
        char c = sessionId[i];
        if(!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
        {
            return false;
        }
    }
    return true;
}

}// namespace nt
