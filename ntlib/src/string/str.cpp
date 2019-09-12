#include "str.h"
#include <algorithm>
#include <cctype>
#include <locale>

namespace nt
{

void split(const std::string &s, const char delim, std::vector<std::string> &out)
{
    std::string::size_type beg = 0;
    for (std::string::size_type end = 0; (end = s.find(delim, end)) != std::string::npos; ++end)
    {
        out.push_back(s.substr(beg, end - beg));
        beg = end + 1;
    }

    out.push_back(s.substr(beg));
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> out;
    split(s, delim, out);
    return out;
}

void replaceAll(std::string &source, const std::string &from, const std::string &to)
{
    std::string newString;
    newString.reserve(source.length());  // avoids a few memory allocations

    std::string::size_type lastPos = 0;
    std::string::size_type findPos;

    while(std::string::npos != (findPos = source.find(from, lastPos)))
    {
        newString.append(source, lastPos, findPos - lastPos);
        newString += to;
        lastPos = findPos + from.length();
    }

    // Care for the rest after last occurrence
    newString += source.substr(lastPos);

    source.swap(newString);
}

static bool NotSpace(int ch)
{
    return !std::isspace(ch);
}

void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), NotSpace));
}

// trim from end (in place)
void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), NotSpace).base(), s.end());
}

// trim from both ends (in place)
void trim(std::string &s)
{
    ltrim(s);
    rtrim(s);
}

}//namespace nt

