#pragma once

#ifdef WITH_BOOST
#include <boost/function.hpp>
#else
#include  <functional>
#endif

namespace nt
{
#ifdef WITH_BOOST
#define NAMESPACE boost
#else
#define NAMESPACE std
#endif

}
