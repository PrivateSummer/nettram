#include "HttpRequest.h"
#include <vector>
#include <string>
#include <string.h>
#include <sstream>
#include "use_shared_ptr.h"
#include "HttpSession.h"
#include "SessionSystem.h"
#include "str.h"

namespace nt
{

HttpRequest::HttpRequest()
{
    m_methodId = UNKNOW;
    m_contentLength = 0;
    m_remotePort = 0;
    m_keepAlive = 0;
    m_version = "1.0";
    m_session = NULL;
}

HttpRequest::~HttpRequest()
{
    if(m_session != NULL)
    {
        delete m_session;
        m_session = NULL;
    }
}

const std::string *HttpRequest::GetHeader(const std::string &key) const
{
    for (size_t i = 0; i < m_headers.size(); i++)
    {
        if (strcasecmp(m_headers[i].key.c_str(), key.c_str()) == 0)
        {
            return &m_headers[i].value;
        }
    }
    return NULL;
}

const std::vector<HttpRequest::header> &HttpRequest::GetHeaders() const
{
    return m_headers;
}

std::vector<HttpRequest::header> &HttpRequest::Headers()
{
    return m_headers;
}

void HttpRequest::ClearHeaders()
{
    m_headers.clear();
}

void HttpRequest::SetMethodId(HTTP_METHOD methodId)
{
    m_methodId = methodId;
}

HTTP_METHOD HttpRequest::GetMethodId() const
{
    return m_methodId;
}

void HttpRequest::SetMethod(const std::string &method)
{
    m_method = method;
}

const std::string &HttpRequest::GetMethod() const
{
    return m_method;
}

std::string &HttpRequest::Method()
{
    return m_method;
}

void HttpRequest::SetRequestURI(const std::string &uri)
{
    m_requestURI = uri;
}

const std::string &HttpRequest::GetRequestURI() const
{
    return m_requestURI;
}

std::string &HttpRequest::RequestURI()
{
    return m_requestURI;
}

void HttpRequest::SetInput(const std::string &input)
{
    m_input = input;
}

const std::string &HttpRequest::GetInput() const
{
    return m_input;
}

std::string *HttpRequest::GetInputImpl()
{
    return &m_input;
}

void HttpRequest::SetRemoteAddr(const std::string &addr)
{
    m_remoteAddr = addr;
}

const std::string &HttpRequest::GetRemoteAddr() const
{
    return m_remoteAddr;
}

void HttpRequest::SetQueryString(const std::string &queryString)
{
    m_queryString = queryString;
}

const std::string &HttpRequest::GetQueryString() const
{
    return m_queryString;
}

void HttpRequest::SetRemotePort(int remotePort)
{
    m_remotePort = remotePort;
}

int HttpRequest::GetRemotePort() const
{
    return m_remotePort;
}

void HttpRequest::SetKeepAlive(int keepAlive)
{
    m_keepAlive = keepAlive;
}

int HttpRequest::GetKeepAlive() const
{
    return m_keepAlive;
}

void HttpRequest::SetContentLength(int contentLength)
{
    m_contentLength = contentLength;
}

int HttpRequest::GetContentLength() const
{
    return m_contentLength;
}

void HttpRequest::SetVersion(const std::string &version)
{
    m_version = version;
}

const std::string &HttpRequest::GetVersion() const
{
    return m_version;
}

const std::string &HttpRequest::GetSessionId() const
{
    if(m_session)
    {
        return m_session->GetSessionId();
    }
    static std::string default_empty_str = "";
    return default_empty_str;
}

HttpSession *HttpRequest::GetSession(bool create /*= false*/)
{
    if(m_session == NULL)
    {
        std::string sessionId = GetCookie("SESSIONID");

        if(sessionId.size() > 0 && ! SessionIdValid(sessionId))
        {
            return NULL;
        }

        if(create)
        {
            m_session = SessionSystem::Instance()->MutableGet(sessionId);
        }
        else
        {
            m_session = SessionSystem::Instance()->Get(sessionId);
        }
    }

    return m_session;
}

std::string HttpRequest::GetCookie(const std::string &key)
{
    const std::string *cookies = GetHeader("Cookie");
    if(cookies != 0)
    {
        std::vector<std::string> kv = split(*cookies, ':');
        for(size_t i = 0; i < kv.size(); i++)
        {
            std::vector<std::string> v = split(kv[i], '=');
            if(v.size() == 2)
            {
                trim(v[0]);
                if(v[0] == "SESSIONID")
                {
                    trim(v[1]);
                    return v[1];
                }
            }
        }
    }
    return "";
}

}
