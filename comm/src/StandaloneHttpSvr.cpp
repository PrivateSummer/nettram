#include "StandaloneHttpSvr.h"

#include <signal.h>
#include <stdio.h>
#include "HttpServerImpl.h"
#include "HandlerInterceptor.h"

#include "log.h"

using namespace nt;

void SignalStop ( int sig )
{
	printf("sig:%d exit!!!\n", sig);
	nt::fatal_log("sig:%d exit!!!", sig);
	exit(0);
	return ;
}

void SignalHandler ( void )
{
	/* /usr/include/bits/signum.h */
	assert ( signal ( SIGPIPE, SIG_IGN ) != SIG_ERR ) ;
	assert ( signal ( SIGALRM, SIG_IGN ) != SIG_ERR ) ;
	assert ( signal ( SIGCHLD, SIG_IGN ) != SIG_ERR ) ;
	assert ( sigset ( SIGINT, SignalStop ) != SIG_ERR ) ;
	assert ( sigset ( SIGTERM, SignalStop ) != SIG_ERR ) ;
	return ;
}

	
StandaloneHttpSvr* StandaloneHttpSvr::m_instance = NULL;

StandaloneHttpSvr *StandaloneHttpSvr::Instance()
{
	if(StandaloneHttpSvr::m_instance == NULL)
	{
		StandaloneHttpSvr::m_instance = new StandaloneHttpSvr();
	}
	return StandaloneHttpSvr::m_instance;
}

StandaloneHttpSvr::StandaloneHttpSvr()
{
	pServerImpl = new HttpServerImpl();
	setbuf(stdout, NULL);
	SignalHandler();
	
	srand(time(0));
}

StandaloneHttpSvr::~StandaloneHttpSvr()
{
	if (pServerImpl)
	{
		delete pServerImpl;
	}
}

int StandaloneHttpSvr::OpenLog(const char *properties, int level)
{
	int ret = 0;
	
    ret = open_log(properties, 3);;
    if( ret != 0)
	{
		printf("server_log.start failed:%d. exit\n", ret);
		abort();
	}
	return 0;
}

void StandaloneHttpSvr::Start(int argc, char **argv)
{
    int ret = pServerImpl->Start();
    if(ret != 0)
	{
		fatal_log("server start failed:%d. exit\n", ret);
	}
}

int StandaloneHttpSvr::AddServiceFactory(ServiceFactory *factory)
{
	return pServerImpl->AddServiceFactory(factory);
}

void StandaloneHttpSvr::AddInterceptor(const std::string &name, HandlerInterceptor *interceptor)
{
	pServerImpl->AddInterceptorMapper(name, interceptor);
}

nt::HttpServerImpl * StandaloneHttpSvr::Impl()
{
	return pServerImpl;
}

void StandaloneHttpSvr::SetConfigFile(const std::string &file)
{
	pServerImpl->SetConfigFile(file);
}
void StandaloneHttpSvr::SetActionMapperFile(const std::string &file)
{
	pServerImpl->SetActionMapperFile(file);
}
