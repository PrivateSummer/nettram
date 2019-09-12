#pragma once

#include <string>
#include <stdexcept>

class ServerException : public std::runtime_error
{
public:

    explicit ServerException(const std::string &_Message) throw()
        : std::runtime_error(_Message)
    {}
    explicit ServerException(const char *_Message) throw()
        : std::runtime_error(_Message)
    {}
};

