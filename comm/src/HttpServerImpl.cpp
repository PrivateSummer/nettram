#include "HttpServerImpl.h"

#include "HttpLogicService.h"
#include "log.h"
#include "CTime.h"
#include "ServerException.h"
#include "ServerConfig.h"
#include "SessionSystem.h"

using namespace nt;

HttpServerImpl::HttpServerImpl()
{
}

HttpServerImpl::~HttpServerImpl()
{
    service_mapper_t::iterator service_itr = m_service_mapper.begin();

    for (; service_itr != m_service_mapper.end(); ++service_itr)

    {

        delete service_itr->second;

    }

    m_service_mapper.clear();

    interceptor_mapper_t::iterator interceptor_itr = m_interceptor_mapper.begin();
    for (; interceptor_itr != m_interceptor_mapper.end(); ++interceptor_itr)
    {
        delete interceptor_itr->second;
    }
    m_interceptor_mapper.clear();
}

int HttpServerImpl::init()
{
    CTime ctime;
    int ret = 0;

    srand(time(NULL));


    ServerConfig conf;



    ret = conf.Load(m_config_file.c_str());

    if (ret != 0)

    {

        err_log("Load (%s) failed:%d", m_config_file.c_str(), ret);
        return -1;

    }


    if(!m_action_mapper_file.empty() && (access(m_action_mapper_file.c_str(), F_OK)) != -1) //文件存在
    {
        ret = m_action_config.Load(m_action_mapper_file.c_str());
        if (ret != 0)
        {
            warn_log("Load(%s) failed:%d", m_action_mapper_file.c_str(), ret);
        }
    }

    HttpOptions(HTTP_OPT_PORT, conf.m_port);
    HttpOptions(HTTP_OPT_BUFF_SIZE, conf.m_buf_size);
    HttpOptions(HTTP_OPT_LISTENQ, conf.m_listenq);
    HttpOptions(HTTP_OPT_QUEUE_SIZE, conf.m_queue_size);
    HttpOptions(HTTP_OPT_PROC_SIZE, conf.m_proc_size);
    HttpOptions(HTTP_OPT_MAX_DATA_LEN, conf.m_max_data_len);
    HttpOptions(HTTP_OPT_READ_TIMEOUT, conf.m_read_timeout);
    HttpOptions(HTTP_OPT_WRITE_TIMEOUT, conf.m_write_timeout);

    SessionSystem::Instance();

    const std::vector<InterceptorData> &interceptors = m_action_config.GetInterceptors();
    for(size_t i = 0; i < interceptors.size(); i++)
    {
        debug_log("interceptor:%s", interceptors[i].interceptor.c_str());
        HandlerInterceptor *interceptor = GetInterceptorMapper(interceptors[i].interceptor);
        if(interceptor == NULL)
        {
            err_log("Can't resolve interceptor (%s)", interceptors[i].interceptor.c_str());
            return -2;
        }
        PutInterceptor(interceptors[i].path, interceptor);
    }

    info_log("server init in %d ms", ctime.getFuncRunTime());
    return 0;
}

void HttpServerImpl::service(nt::SharedHttpInput_t request, nt::SharedHttpOutput_t response)
{
    ServiceFactory *serviceFactory = NULL;



    const std::string &uri = request->GetRequestURI();



    std::map<std::string, ActionData>::const_iterator action_itr = m_action_config.GetUriMapper().find(uri);

    if (action_itr != m_action_config.GetUriMapper().end())

    {

        int forbidden = action_itr->second.forbidden;
        if(forbidden == 1)
        {
            response->SetHeader("Content-Length", response->GetOutput().size());
            response->SetStatus(HTTP_FORBIDDEN);

            err_log("403 forbidden uri:%s", uri.c_str());
            return;
        }

    }


    service_mapper_t::const_iterator service_itr = m_service_mapper.find(uri);
    if (service_itr != m_service_mapper.end())
    {
        serviceFactory = service_itr->second;;
    }


    if (serviceFactory == NULL)

    {

        response->GetOutputImpl()->append("not found");

        response->SetHeader("Content-Length", response->GetOutput().size());

        response->SetStatus(HTTP_NOT_FOUND);



        err_log("not found uri:%s", uri.c_str());

        return;

    }



    HttpLogicService *service = static_cast<HttpLogicService *>(serviceFactory->Create());


    response->SetStatus(HTTP_OK);


    service->Process(request.get(), response.get());


    if(response->GetOutput().size() < 8192)
    {
        debug_log("resp:%s", response->GetOutput().c_str());
    }


    debug_log("resp size:%d", (int)response->GetOutput().size());
}

int HttpServerImpl::AddServiceFactory(nt::ServiceFactory *factory)

{

    std::string url = factory->Name();

    if (m_service_mapper.find(url) != m_service_mapper.end())

    {

        printf("AddServiceFactory service(%s) existed\n", url.c_str());

        return -1;

    }

    m_service_mapper.insert(std::make_pair(url, factory));

    AddUrl(url);

    printf("AddServiceFactory cgi:%s\n", url.c_str());

    return 0;

}

int HttpServerImpl::AddInterceptorMapper(const std::string &name, HandlerInterceptor *interceptor)
{
    if (m_interceptor_mapper.find(name) != m_interceptor_mapper.end())
    {
        printf("add_interceptor_mapper name(%s) existed\n", name.c_str());
        return -1;
    }
    m_interceptor_mapper.insert(std::make_pair(name, interceptor));
    printf("add_interceptor_mapper name:%s\n", name.c_str());
    return 0;
}

HandlerInterceptor *HttpServerImpl::GetInterceptorMapper(const std::string &name)
{
    HandlerInterceptor *interceptor = NULL;
    interceptor_mapper_t::iterator itr = m_interceptor_mapper.find(name);
    if (itr != m_interceptor_mapper.end())
    {
        interceptor = itr->second;
    }
    return interceptor;
}

void HttpServerImpl::SetConfigFile(const std::string &file)
{
    m_config_file = file;
}
void HttpServerImpl::SetActionMapperFile(const std::string &file)
{
    m_action_mapper_file = file;
}
