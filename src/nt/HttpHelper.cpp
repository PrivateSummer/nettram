#include "HttpHelper.h"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "log.h"
#include "CTime.h"

namespace nt
{

HttpHelper::HttpHelper(SharedHttpInput_t input)
{
    m_input = input;
}
HttpHelper::~HttpHelper()
{

}

struct InfoToCode
{
    const char *info;
    int code;
};

char *HttpHelper::GetKeyValue(char *&value)
{
    char *sign = NULL;
    while (*value != '\0')
    {
        if (*value == ':')
        {
            *value = '\0';
            sign = value;
            value++;
            break;
        }
        value++;
    }

    while (isspace(*value) && *value != '\0')
    {
        value++;
    }
    return sign;
}

int HttpHelper::rtrim(char *s, int len)
{
    int i = len - 1;
    for (; i >= 0 && (isspace(s[i])); --i)
    {
        s[i] = '\0';
    }
    return i + 1;
}

int HttpHelper::GetLine(int sock, char *line, int size)
{
    int i = 0;
    char c = '\0';
    int n;

    // 处理字符串以\n结尾
    while ((i < size - 1) && (c != '\n'))
    {
        n = recv(sock, &c, 1, 0);

        if (n > 0)
        {
            if (c == '\r')
            {
                n = recv(sock, &c, 1, MSG_PEEK);

                if ((n > 0) && (c == '\n'))
                {
                    if ((n = recv(sock, &c, 1, 0)) < 0)
                    {
                        info_log("recv: errno:%d, %s", errno, strerror(errno));
                        break;
                    }
                }
                else
                {
                    c = '\n';
                }
            }

            line[i] = c;
            i++;
        }
        else
        {
            info_log("recv: errno:%d, %s", errno, strerror(errno));
            c = '\n';
        }
    }
    line[i] = '\0';

    //返回数据长度
    return(i);
}

int HttpHelper::ReadData(int client, int buf_size, int max_data_len)
{
    CTime ctime;
    char line[1024];
    int numchars = 0;

    numchars = GetLine(client, line, sizeof(line));
    if(numchars >= static_cast<int>(sizeof(line) - 1))
    {
        err_log("414 URI Too Long");
        return HTTP_URI_TOO_LONG;
    }

    // method
    int methodId = UNKNOW;
    char method[255];
    size_t i = 0;
    size_t j = 0;
    while (!isspace(line[j]) && (i < (int)(sizeof(method) - 1)))
    {
        method[i] = line[j];
        i++;
        j++;
    }
    method[i] = '\0';
    if (strcasecmp(method, "GET") == 0)
    {
        methodId = GET;
    }
    else if (strcasecmp(method, "POST") == 0)
    {
        methodId = POST;
    }

    // 不支持的方法
    if (methodId == UNKNOW)
    {
        if(numchars > 0)
        {
            err_log("501 method is unknow. line:{%s}", line);
        }
        else
        {
            info_log("recv line empty within the timeout");
        }
        return HTTP_METHOD_NOT_IMPLEMENTED;
    }
    m_input->SetMethod(method);
    m_input->SetMethodId((nt::HTTP_METHOD)methodId);

    // url
    char url[1024];
    i = 0;
    while (isspace(line[j]) && (j < sizeof(line)))
    {
        j++;
    }
    while (!isspace(line[j]) && (i < (int)(sizeof(url) - 1)) && (j < sizeof(line)))
    {
        url[i] = line[j];
        i++;
        j++;
    }
    url[i] = '\0';

    // 请求串
    char *query_string = url;

    while ((*query_string != '?') && (*query_string != '\0'))
        query_string++;
    // ? 后面为请求串
    if (*query_string == '?')
    {
        *query_string = '\0';
        query_string++;
    }

    char *request_uri = url;

    m_input->SetRequestURI(request_uri);

    if (query_string)
    {
        m_input->SetQueryString(query_string);
    }

    int content_length = -1;

    numchars = GetLine(client, line, sizeof(line));
    for (int index = 0; (numchars > 0) && strcmp("\n", line); index++)
    {
        if(numchars >= static_cast<int>(sizeof(line) - 1) || index > MAX_HEADER_LINE_COUNTS)
        {
            err_log("431 Request Header Fields Too Large");
            return HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE;
        }

        char *value = line;

        if (GetKeyValue(value))
        {
            int valuelen = rtrim(value, strlen(value));
            m_input->SetHeader(line, std::string(value, valuelen));

            if (strcasecmp(line, "Connection") == 0)
            {
                m_input->SetKeepAlive((strcasecmp(value, "keep-alive") == 0));
            }
            else if (methodId == POST && strcasecmp(line, "Content-Length") == 0)// POST请求需要 Content-Length
            {
                content_length = atoi(value);
                m_input->SetContentLength(content_length);
            }
        }
        numchars = GetLine(client, line, sizeof(line));
    }
    debug_log("parse header runtime:%d ms", ctime.getFuncRunTime());
    if (methodId == POST)
    {
        if (content_length < 0)
        {
            // 400 错误请求
            err_log("400 Bad Request");
            return HTTP_BAD_REQUEST;
        }

        if(max_data_len > 0 && content_length > max_data_len)
        {
            // 413 请求实体过长
            err_log("413 Payload Too Large");
            return HTTP_PAYLOAD_TOO_LARGE;
        }

        m_input->GetInputImpl()->resize(content_length);

        // 接收 body
        for (int i = 0; i < content_length; )
        {
            int recv_ret = -1;
            if ((recv_ret = recv(client, (char *)m_input->GetInputImpl()->data() + i, std::min((int)buf_size, content_length - i), 0)) < 0)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                if (errno == EWOULDBLOCK)
                {
                    continue;
                }
                err_log("recv: %d.%s", errno, strerror(errno));
                return HTTP_READ;
            }

            i += recv_ret;
        }
    }
    debug_log("parse body runtime:%d ms", ctime.getFuncRunTime());
    return HTTP_SUCCESS;
}

}// namespace nt
