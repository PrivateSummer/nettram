#include "ServiceMapper.h"
#include <assert.h>
#include <string.h>
#include "log.h"

using namespace nt;

class echo: public HttpLogicService
{
public:
    virtual void Process(HttpRequest *request, HttpResponse *response)
    {
        const std::string &req = request->GetQueryString();

        response->SetOutput(req.c_str());
    }
};
NT_RequestMapping(echo, "/demo/echo")
