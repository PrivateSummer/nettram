#pragma once
#ifdef WITH_BOOST
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/weak_ptr.hpp>
#else 
#include <memory>
#endif

namespace nt
{
#ifdef WITH_BOOST
using boost::make_shared;
#else 
using std::make_shared;
#endif

}
