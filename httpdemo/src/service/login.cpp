#include "ServiceMapper.h"
#include <assert.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include "log.h"

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
                debug_log("login already, %s", username->c_str());
                response->SetOutput("OK\n");
                return;
            }

            session = request->GetSession(true);

            if(session)
            {
                session->SetAttribute("username", req);
                response->SetOutput("OK\n");
                return;
            }
        }
        usleep(20000);
        response->SetOutput("username or password is wrong\n");
    }
};
NT_RequestMapping(login, "/login")
