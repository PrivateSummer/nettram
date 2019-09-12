#include "log.h"

namespace nt
{

nt::simpleLog::Log *g_p_default_Log = NULL;

int open_log(const char *file, int level)
{
	int ret = 0;
	
	static nt::simpleLog::Log server_log(file);
    g_p_default_Log = &server_log;
	server_log.setLogLevel(level);
	ret = server_log.start();
    if( ret != 0)
	{
		printf("server_log.start failed:%d\n", ret);
	}
	return ret;
}	
	
} //namespace nt

