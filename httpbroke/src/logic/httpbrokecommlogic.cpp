#include "httpbrokecommlogic.h"

std::string GenResp(int ret, const char *info)
{
    char buf[4096] = {0};
    snprintf(buf, sizeof(buf), "{ \"resp\": { \"ret\": %d, \"info\": \"%s\" } }\n", ret, info ? info : "");

    return buf;
}

