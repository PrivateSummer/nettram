#pragma once

#include <map>
#include <string>
#include <list>
#include <sstream>

#include "ServiceFactory.h"
#include "ActionMapperConfig.h"
#include "HttpServer.h"

namespace nt
{

class HttpServerImpl : public HttpServer
{
public:
	HttpServerImpl();
	~HttpServerImpl();

	

	virtual void service(nt::SharedHttpInput_t request, nt::SharedHttpOutput_t response);

	int AddServiceFactory(nt::ServiceFactory* factory);
	int AddInterceptorMapper(const std::string &name, HandlerInterceptor *interceptor);
	
	void SetConfigFile(const std::string &file);
	void SetActionMapperFile(const std::string &file);
	
	HandlerInterceptor *GetInterceptorMapper(const std::string &name);
protected:
	virtual int init();
private:
	typedef std::map<std::string, nt::ServiceFactory*> service_mapper_t;
	typedef std::map<std::string, nt::HandlerInterceptor*> interceptor_mapper_t;
	service_mapper_t m_service_mapper;
	nt::ActionMapperConfig m_action_config;
	std::string m_config_file;
	std::string m_action_mapper_file;
	interceptor_mapper_t m_interceptor_mapper;
}; // HttpServerImpl

}// namespace nt
