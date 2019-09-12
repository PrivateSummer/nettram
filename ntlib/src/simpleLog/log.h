#pragma once

#include "SimpleLog.h"

namespace nt
{
int open_log(const char *file, int level);

extern nt::simpleLog::Log *g_p_default_Log;

#define debug_log(format,...) g_p_default_Log->print_log(nt::simpleLog::LOG_LELEVE_DEBUG,"%s:%d[%s] [DEBUG] "format,__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define info_log(format,...) g_p_default_Log->print_log(nt::simpleLog::LOG_LELEVE_INFO,"%s:%d[%s] [INFO] "format,__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define warn_log(format,...) g_p_default_Log->print_log(nt::simpleLog::LOG_LELEVE_WARNING,"%s:%d[%s] [WARN] "format,__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define err_log(format,...) g_p_default_Log->print_log(nt::simpleLog::LOG_LELEVE_ERROR,"%s:%d[%s] [ERR] "format,__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define fatal_log(format,...) g_p_default_Log->print_log(nt::simpleLog::LOG_LELEVE_FATAL,"%s:%d[%s] [FATAL] "format,__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)

} //namespace nt
