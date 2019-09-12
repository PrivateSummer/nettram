#pragma once

#pragma once
#ifdef WITH_BOOST
#include <boost/thread/mutex.hpp>
#else 
#include <mutex>
#endif

namespace nt
{
#ifdef WITH_BOOST
using boost::mutex;
#else 
using std::mutex;
#endif

}
