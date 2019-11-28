#pragma once

#include <string>
#include "rapidxml/rapidxml.hpp"

namespace nt
{

class ServerConfig
{
public:
    int Load(const char *file);

    void ClearAll();

    int ReadData(const char *file, rapidxml::xml_node<> *root);
public:
    std::string m_ip;
    int m_port;
    int m_buf_size;
    int m_listenq;

    int m_proc_size;
    int m_queue_size;

    int m_max_data_len;

    int m_read_timeout;
    int m_write_timeout;
};

}
