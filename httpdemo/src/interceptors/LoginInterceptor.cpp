#include <vector>
#include "HandlerInterceptor.h"
#include "ServiceMapper.h"
#include "log.h"

using namespace nt;

class LoginInterceptor : public nt::HandlerInterceptor
{
public:
    virtual bool PreHandle(HttpRequest *request, HttpResponse *response, void *handler)
    {
        const std::vector<HttpRequest::header> &headers = request->GetHeaders();
        for(size_t i = 0; i < headers.size(); i++)
        {
            debug_log("header>>> %s: %s", headers[i].key.c_str(), headers[i].value.c_str());
        }

        const std::string *username = NULL;
        HttpSession *session = request->GetSession(false);
        if(session != NULL && (username = session->GetAttribute("username")) != NULL)
        {
            debug_log("login already, %s", username->c_str());
            return true;
        }
        else
        {
            debug_log("not login");
            response->SetStatus(401);
            response->SetOutput("Unauthorized");
            return false;
        }

        return true;
    }
};

NT_InterceptorMapping(LoginInterceptor);
