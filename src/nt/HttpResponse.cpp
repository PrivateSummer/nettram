#include "HttpResponse.h"
#include <vector>
#include <string>
#include <string.h>
#include <sstream>
#include "use_shared_ptr.h"
#include "HttpStatus.h"

namespace nt
{

HttpResponse::HttpResponse()
{
    m_contentLength = 0;
    m_status = 200;
    m_version = "1.0";
}
HttpResponse::~HttpResponse()
{}

const std::string *HttpResponse::GetHeader(const std::string &key) const
{
    const HttpResponse::header *h = Header(key);
    if(h)
    {
        return &(h->value);
    }

    return NULL;
}

const std::vector<HttpResponse::header> &HttpResponse::GetHeaders() const
{
    return m_headers;
}

std::vector<HttpResponse::header> &HttpResponse::Headers()
{
    return m_headers;
}

void HttpResponse::ClearHeaders()
{
    m_headers.clear();
}

void HttpResponse::SetOutput(const std::string &output)
{
    m_output = output;
}

const std::string &HttpResponse::GetOutput() const
{
    return m_output;
}

std::string *HttpResponse::GetOutputImpl()
{
    return &m_output;
}

void HttpResponse::SetContentLength(int contentLength)
{
    m_contentLength = contentLength;
}

int HttpResponse::GetContentLength() const
{
    return m_contentLength;
}

void HttpResponse::SetStatus(int status)
{
    m_status = status;
}

int HttpResponse::GetStatus() const
{
    return m_status;
}

void HttpResponse::SetVersion(const std::string &version)
{
    m_version = version;
}

std::string HttpResponse::GetVersion() const
{
    return m_version;
}

void HttpResponse::SendRedirect(const std::string &url)
{
    SetStatus(HTTP_FOUND);
    SetHeader("Location", url);
}

void HttpResponse::SetContentType(const std::string &contentType)
{
    SetHeader("Content-Type", contentType);
}

const HttpResponse::header *HttpResponse::Header(const std::string &key) const
{
    for (size_t i = 0; i < m_headers.size(); i++)
    {
        if (strcasecmp(m_headers[i].key.c_str(), key.c_str()) == 0)
        {
            return &m_headers[i];
        }
    }
    return NULL;
}

}
