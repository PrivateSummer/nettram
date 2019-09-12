#pragma once

#include "HttpResponse.h"
#include "HttpStatus.h"

namespace nt
{

class HttpRespHelper
{
public:
    HttpRespHelper(SharedHttpOutput_t output);
    ~HttpRespHelper();

    void CreateHeader(std::string &header);

private:
    SharedHttpOutput_t m_output;
};

};

