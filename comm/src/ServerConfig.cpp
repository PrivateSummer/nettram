#include "ServerConfig.h"

#include <stdio.h>
#include "log.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"

namespace nt
{
int ServerConfig::Load(const char *file)
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
    catch(std::exception &ex)
    {
        err_log("file:%s :%s\n", file, ex.what());
        return -1;
    }
}

int ServerConfig::ReadData(const char *file, rapidxml::xml_node<> *root)
{
    rapidxml::xml_node<> *server = root->first_node("server");
    if(server == NULL)
    {
        err_log("read(%s): no such node (server)", file);
        return 1;
    }

    rapidxml::xml_node<> *node = NULL;

    node = server->first_node("ip");
    if(node == NULL)
    {
        err_log("read(%s): no such node (ip)", file);
        return 1;
    }
    m_ip = node->value();

    node = server->first_node("port");
    if(node == NULL)
    {
        err_log("read(%s): no such node (port)", file);
        return 1;
    }
    m_port = atoi(node->value());


    node = server->first_node("buf_size");
    if(node == NULL)
    {
        err_log("read(%s): no such node (buf_size)", file);
        return 1;
    }
    m_buf_size = atoi(node->value());

    node = server->first_node("max_events");
    if(node == NULL)
    {
        err_log("read(%s): no such node (max_events)", file);
        return 1;
    }
    m_max_events = atoi(node->value());

    node = server->first_node("listenq");
    if(node == NULL)
    {
        err_log("read(%s): no such node (listenq)", file);
        return 1;
    }
    m_listenq = atoi(node->value());

    node = server->first_node("data_queue_max_size");
    if(node == NULL)
    {
        err_log("read(%s): no such node (data_queue_max_size)", file);
        return 1;
    }
    m_data_queue_max_size = atoi(node->value());

    node = server->first_node("proc_size");
    if(node == NULL)
    {
        err_log("read(%s): no such node (proc_size)", file);
        return 1;
    }
    m_proc_size = atoi(node->value());

    node = server->first_node("queue_size");
    if(node == NULL)
    {
        err_log("read(%s): no such node (queue_size)", file);
        return 1;
    }
    m_queue_size = atoi(node->value());

    node = server->first_node("max_data_len");
    if(node == NULL)
    {
        err_log("read(%s): no such node (max_data_len)", file);
        return 1;
    }
    m_max_data_len = atoi(node->value());

    node = server->first_node("read_timeout");
    if(node == NULL)
    {
        warn_log("read(%s): no such node (read_timeout)", file);
        m_read_timeout = 10;
    }
    else
    {
        m_read_timeout = atoi(node->value());
    }

    node = server->first_node("write_timeout");
    if(node == NULL)
    {
        warn_log("read(%s): no such node (write_timeout)", file);
        m_write_timeout = 10;
    }
    else
    {
        m_write_timeout = atoi(node->value());
    }

    return 0;
}

void ServerConfig::ClearAll()
{
    m_ip.clear();
    m_port = 0;
    m_buf_size = 0;
    m_max_events = 0;
    m_listenq = 0;

    m_data_queue_max_size = 0;

    m_proc_size = 0;
    m_queue_size = 0;

    m_read_timeout = 10;
    m_write_timeout = 10;
}
}
