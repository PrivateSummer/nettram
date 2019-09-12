#pragma once

#ifdef WITH_BOOST
#include <boost/thread/thread.hpp>
#else 
#include <thread>
#endif

namespace nt
{
#ifdef WITH_BOOST
using boost::thread;
#else 
using std::thread;
#endif

}
