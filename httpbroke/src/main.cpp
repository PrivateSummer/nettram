#include <signal.h>
#include <stdio.h>
#include "StandaloneHttpSvr.h"
#include "ServiceMapper.h"

using namespace std;
using namespace nt;

int main(int argc, char **argv)
{
	StandaloneHttpSvr::Instance()->OpenLog("./log.properties", 3);
	
    StandaloneHttpSvr::Instance()->SetConfigFile("web.xml");
	StandaloneHttpSvr::Instance()->SetActionMapperFile("action.xml");
	
	StandaloneHttpSvr::Instance()->Start(argc, argv);
	return 0;
}
