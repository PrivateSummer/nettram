#pragma once

#include <vector>
#include <string>
#include <string.h>
#include <sstream>
#include "use_shared_ptr.h"
#include "HttpSession.h"

namespace nt
{

enum HTTP_METHOD
{
    UNKNOW = 0,
    GET = 1,
    POST = 2
};

class HttpRequest
{
public:
    HttpRequest();
    virtual ~HttpRequest();


    struct header
    {
        std::string key;
        std::string value;
    };

    template<typename T>
    void SetHeader(const std::string &key, const T &value)
    {
        std::stringstream ss;
        ss << value;
        header h = { key, ss.str() };

        m_headers.push_back(h);
    }

    const std::string *GetHeader(const std::string &key) const;

    const std::vector<HttpRequest::header> &GetHeaders() const;

    std::vector<HttpRequest::header> &Headers();

    void ClearHeaders();

    void SetMethodId(HTTP_METHOD methodId);

    HTTP_METHOD GetMethodId() const;

    void SetMethod(const std::string &method);

    const std::string &GetMethod() const;

    std::string &Method();

    void SetRequestURI(const std::string &uri);

    const std::string &GetRequestURI() const;

    std::string &RequestURI();

    void SetInput(const std::string &input);

    const std::string &GetInput() const;

    std::string *GetInputImpl();

    void SetRemoteAddr(const std::string &addr);

    const std::string &GetRemoteAddr() const;

    void SetQueryString(const std::string &queryString);

    const std::string &GetQueryString() const;

    void SetRemotePort(int remotePort);

    int GetRemotePort() const;

    void SetKeepAlive(int keepAlive);

    int GetKeepAlive() const;

    void SetContentLength(int contentLength);

    int GetContentLength() const;

    void SetVersion(const std::string &version);

    const std::string &GetVersion() const;

    const std::string &GetSessionId() const;

    HttpSession *GetSession(bool create = false);
    
    std::string GetCookie(const std::string &key);
private:
    std::vector<HttpRequest::header> m_headers;
    int m_contentLength;
    HTTP_METHOD m_methodId;
    std::string m_requestURI;
    std::string m_input;
    std::string m_remoteAddr;
    int m_remotePort;
    std::string m_queryString;
    std::string m_method;
    int m_keepAlive;
    std::string m_version;
    HttpSession *m_session;
};

#ifdef WITH_BOOST
typedef boost::shared_ptr<nt::HttpRequest> SharedHttpInput_t;
#else
typedef std::shared_ptr<nt::HttpRequest> SharedHttpInput_t;
#endif
#define CreateSharedHttpInput() make_shared<nt::HttpRequest>();

}
