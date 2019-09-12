#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"
namespace nt
{

class HandlerInterceptor
{
public:
    HandlerInterceptor() {};
    virtual ~HandlerInterceptor() {};

    virtual bool PreHandle(HttpRequest *request, HttpResponse *response, void *handler)
    {
        return true;
    };
    virtual void AfterCompletion(HttpRequest *request, HttpResponse *response, void *handler, const char *what) {};
};

};

