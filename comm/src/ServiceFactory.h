#pragma once

#include "LogicService.h"

namespace nt
{

class ServiceFactory
{
public:
    virtual LogicService *Create() = 0;
    virtual const char *Name() = 0;

    virtual ~ServiceFactory() {};
};

}
