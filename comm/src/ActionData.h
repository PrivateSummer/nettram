#pragma once

#include <string>
#include <vector>

namespace nt
{

struct ActionData
{
    int req;
    int resp;
	std::string uri;
	int forbidden;
};

struct InterceptorData
{
	std::vector<std::string> path;
	std::string interceptor;
};

}
