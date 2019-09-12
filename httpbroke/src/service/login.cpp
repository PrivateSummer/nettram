#include "ServiceMapper.h"
#include <assert.h>
#include <string.h>
#include "log.h"
#include "httpbrokecommlogic.h"

using namespace nt;

class login: public HttpLogicService
{
public:
    virtual void Process(HttpRequest *request, HttpResponse *response)
    {
        const std::string &req = request->GetQueryString();
        if(req == "xx")
        {
            const std::string *username = NULL;
            HttpSession *session = request->GetSession(false);
            if(session != NULL && (username = session->GetAttribute("username")) != NULL)
            {
                debug_log("login already, %s", username);
                response->SetOutput(GenResp(0, "OK"));
                return;
            }

            session = request->GetSession(true);

            if(session)
            {
                session->SetAttribute("username", req);
                response->SetOutput(GenResp(0, "OK"));
                return;
            }
        }

        response->SetOutput(GenResp(-1, "username or password is wrong"));
    }
};
NT_RequestMapping(login, "/login")
