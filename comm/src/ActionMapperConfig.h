#pragma once

#include <map>
#include <string>
#include "ActionData.h"
#include "rapidxml/rapidxml.hpp"

namespace nt
{

class ActionMapperConfig
{
public:
    int Load(const char *file);
public:
    const std::map<std::string, ActionData> &GetUriMapper() const;
    const std::vector<InterceptorData> &GetInterceptors() const;
private:
    int AddAction(const ActionData &actionData);

    int ReadData(const char *file, rapidxml::xml_node<> *root);
    void ClearAll();

    std::map<std::string, ActionData> m_uri_mapper;

    std::vector<InterceptorData> m_interceptors;
};

}
