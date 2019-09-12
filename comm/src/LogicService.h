#pragma once

#include "LogicInput.h"
#include <string>
namespace nt
{

class LogicService
{
public:
    virtual void Init() {};

    virtual const char *ServiceName() const = 0;

    virtual ~LogicService() {};
};

}
