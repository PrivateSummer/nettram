#include "ActionMapperConfig.h"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include <stdio.h>
#include "log.h"

namespace nt
{
int ActionMapperConfig::Load(const char *file)
{
    try
    {
        rapidxml::file<> fdoc(file);
        rapidxml::xml_document<> doc;
        doc.parse<0>(fdoc.data());
        rapidxml::xml_node<> *root = doc.first_node("config");

        ClearAll();
        return ReadData(file, root);
    }
    catch (std::exception &ex)
    {
        err_log("conf.load file:%s error:%s\n", file, ex.what());
        printf("conf.load file:%s error:%s\n", file, ex.what());
        return -1;
    }
}

int ActionMapperConfig::ReadData(const char *file, rapidxml::xml_node<> *root)
{
    //启用或禁止接口
    rapidxml::xml_node<> *action_mapper = root->first_node("action_mapper");

    for( ; action_mapper; action_mapper = action_mapper->next_sibling())
    {
        if(action_mapper->name() == std::string("action_mapper"))
        {
            rapidxml::xml_attribute<char> *attr = NULL;
            ActionData actionData;

            attr = action_mapper->first_attribute("request");
            actionData.req = attr ? atoi(attr->value()) : 0;

            attr = action_mapper->first_attribute("response");
            actionData.resp = attr ? atoi(attr->value()) : 0;

            attr = action_mapper->first_attribute("forbidden");
            actionData.forbidden = attr ? atoi(attr->value()) : 0;

            attr = action_mapper->first_attribute("uri");
            actionData.uri = attr ? attr->value() : "";

            AddAction(actionData);
        }
    }

    //拦截器
    rapidxml::xml_node<> *interceptors = root->first_node("interceptors");
    if(interceptors)
    {
        rapidxml::xml_node<> *interceptor = interceptors->first_node("interceptor");
        for( ; interceptor; interceptor = interceptor->next_sibling())
        {
            InterceptorData data;

            rapidxml::xml_node<> *mapping = interceptor->first_node("mapping");
            for( ; mapping; mapping = mapping->next_sibling())
            {
                if(mapping->name() == std::string("mapping"))
                {
                    rapidxml::xml_attribute<char> *attr = mapping->first_attribute("path");
                    if(attr == NULL)
                    {
                        err_log("read(%s): no such attribute (path). should like '<mapping path=\"/xx/.*\"/>", file);
                        return 1;
                    }
                    data.path.push_back(attr->value());
                }
            }
            if(data.path.empty())
            {
                err_log("read(%s): data.path.empty. should like '<mapping path=\"/xx/.*\"/>", file);
                return 1;
            }
            rapidxml::xml_node<> *className = interceptor->first_node("class");
            if(className == NULL)
            {
                err_log("read(%s): no such node (class). should like '<class name=\"xx\"></class>'", file);
                return 1;
            }
            rapidxml::xml_attribute<char> *attr = className->first_attribute("name");
            if(attr == NULL)
            {
                err_log("read(%s): no such attribute (name). should like '<class name=\"xx\"></class>'", file);
                return 1;
            }
            data.interceptor = attr->value();

            m_interceptors.push_back(data);
        }
    }

    return 0;
}

void ActionMapperConfig::ClearAll()
{
    m_uri_mapper.clear();
    m_interceptors.clear();
}

int ActionMapperConfig::AddAction(const ActionData &actionData)
{
    std::map<std::string, ActionData>::const_iterator itr = m_uri_mapper.find(actionData.uri);
    if(itr != m_uri_mapper.end())
    {
        warn_log("uri:%s already added", actionData.uri.c_str());
        return 1;
    }
    m_uri_mapper.insert(std::make_pair(actionData.uri, actionData));

    return 0;
}

const std::map<std::string, ActionData> &ActionMapperConfig::GetUriMapper() const
{
    return m_uri_mapper;
}

const std::vector<InterceptorData> &ActionMapperConfig::GetInterceptors() const
{
    return m_interceptors;
}
}
