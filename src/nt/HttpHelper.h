#pragma once

#include "HttpRequest.h"
#include "HttpStatus.h"
namespace nt
{

#define MAX_HEADER_LINE_COUNTS 50

enum HttpPaserError
{
    HTTP_READ = -1,
    HTTP_SUCCESS = 0,
};

class HttpHelper
{
public:
    HttpHelper(SharedHttpInput_t input);
    ~HttpHelper();

    int ReadData(int client, int buf_size, int max_data_len);
private:
    int GetLine(int sock, char *buf, int size);

    int rtrim(char *s, int len);
    char *GetKeyValue(char *&value);

    SharedHttpInput_t m_input;
};

};

