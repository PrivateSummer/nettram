#pragma once

#include <vector>
#include <string>
#include <string.h>
#include <sstream>
#include "use_shared_ptr.h"
#include "HttpStatus.h"

namespace nt
{

class HttpResponse
{
public:
    HttpResponse()
    {
        m_contentLength = 0;
        m_status = 200;
        m_version = "1.0";
    }
    virtual ~HttpResponse() {};


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

    const std::string *GetHeader(const std::string &key) const
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

    const std::vector<HttpResponse::header> &GetHeaders() const
    {
        return m_headers;
    }

    std::vector<HttpResponse::header> &Headers()
    {
        return m_headers;
    }

    void ClearHeaders()
    {
        m_headers.clear();
    }

    void SetOutput(const std::string &output)
    {
        m_output = output;
    }

    const std::string &GetOutput() const
    {
        return m_output;
    }

    std::string *GetOutputImpl()
    {
        return &m_output;
    }

    void SetContentLength(int contentLength)
    {
        m_contentLength = contentLength;
    }

    int GetContentLength() const
    {
        return m_contentLength;
    }

    void SetStatus(int status)
    {
        m_status = status;
    }

    int GetStatus() const
    {
        return m_status;
    }

    void SetVersion(const std::string &version)
    {
        m_version = version;
    }

    std::string GetVersion() const
    {
        return m_version;
    }

    void SendRedirect(const std::string &url)
    {
        SetStatus(HTTP_FOUND);
        SetHeader("Location", url);
    }
private:
    std::vector<HttpResponse::header> m_headers;
    int m_contentLength;

    std::string m_output;

    int m_status;
    std::string m_version;
};

#ifdef WITH_BOOST
typedef boost::shared_ptr<nt::HttpResponse> SharedHttpOutput_t;
#else
typedef std::shared_ptr<nt::HttpResponse> SharedHttpOutput_t;
#endif
#define CreateSharedHttpOutput() make_shared<nt::HttpResponse>();

}
