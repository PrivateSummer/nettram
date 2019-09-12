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
    HttpResponse();
    
    virtual ~HttpResponse();


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

    const std::vector<HttpResponse::header> &GetHeaders() const;

    std::vector<HttpResponse::header> &Headers();

    void ClearHeaders();

    void SetOutput(const std::string &output);

    const std::string &GetOutput() const;

    std::string *GetOutputImpl();
    
    void SetContentLength(int contentLength);

    int GetContentLength() const;

    void SetStatus(int status);

    int GetStatus() const;

    void SetVersion(const std::string &version);

    std::string GetVersion() const;
    
    void SendRedirect(const std::string &url);
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
