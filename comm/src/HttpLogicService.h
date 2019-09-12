#pragma once

#include "LogicService.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
#include <string>
namespace nt
{

class HttpLogicService : public LogicService
{
public:
    virtual void Process(HttpRequest *request, HttpResponse *response) = 0;
    const char *ServiceName() const
    {
        return m_name.c_str();
    }
    void SetServiceName(const std::string &name)
    {
        m_name = name;
    }
private:
    std::string m_name;
};

}
