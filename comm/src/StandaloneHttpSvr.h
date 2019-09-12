#pragma once

#include "LogicService.h"
#include "ServiceFactory.h"

namespace nt
{
class HttpServerImpl;
class HandlerInterceptor;

class StandaloneHttpSvr
{
public:
	static StandaloneHttpSvr *Instance();
	~StandaloneHttpSvr();
	int OpenLog(const char *properties, int level);
	
	void Start(int argc, char **argv);
	
	int AddServiceFactory(nt::ServiceFactory *factory);
	void AddInterceptor(const std::string &name, HandlerInterceptor *interceptor);
	nt::HttpServerImpl *Impl();
	
	void SetConfigFile(const std::string &file);
	void SetActionMapperFile(const std::string &file);
private:
	StandaloneHttpSvr();
	static StandaloneHttpSvr* m_instance;
    nt::HttpServerImpl* pServerImpl;
};

}
