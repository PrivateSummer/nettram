#ifdef WITH_BOOST
#include <boost/unordered_map.hpp>
#else
#include <unordered_map>
#endif

namespace nt
{
#ifdef WITH_BOOST
using boost::unordered_map;
#else
using std::unordered_map;
#endif

}
