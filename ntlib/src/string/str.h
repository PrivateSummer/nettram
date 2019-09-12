#pragma once

#include <string>
#include <vector>

namespace nt
{
void split(const std::string &s, const char delim, std::vector<std::string> &out);

std::vector<std::string> split(const std::string &s, char delim);

void replaceAll(std::string &source, const std::string &from, const std::string &to);

void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);

} //namespace nt
