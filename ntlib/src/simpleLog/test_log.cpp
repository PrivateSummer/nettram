#ifdef XX_TEST_LOG
#include <iostream>
#include <unistd.h>
#include "log.h"

nt::simpleLog::Log *g_p_default_Log = NULL;
int main()
{
    setbuf(stdout, NULL);
#if 1
    static nt::simpleLog::Log server_log("./simple.properties");
#else
    static nt::simpleLog::Log server_log("stdout");
#endif
    g_p_default_Log = &server_log;

    server_log.setRoll(2);
    server_log.setLogLevel(5);

    server_log.start();



    debug_log("this is a(n) debug log");
    info_log("this is a(n) info log");
    warn_log("this is a(n) warn log");
    err_log("this is a(n) error log");
    fatal_log("this is a(n) fatal log");

#if 1
    int d = 12;
    for(;;)
    {
        info_log("this is a(n) info log aaa:%d", d);
        sleep(1);
    }
#endif
    return 0;
}

#endif // #ifdef XX_TEST_LOG

/*
file
time ./test_log

real	0m0.043s
user	0m0.020s
sys	0m0.022s

stdout
real	0m0.383s
user	0m0.000s
sys	0m0.005s

real	0m0.377s
user	0m0.002s
sys	0m0.011s

*/

