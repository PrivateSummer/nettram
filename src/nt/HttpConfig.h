#pragma once

#include <string>
namespace nt
{

class HttpConfig
{
public:
    std::string m_ip;
    int m_port;
    int m_buf_size;
    int m_max_events;
    int m_listenq;

    int m_data_queue_max_size;

    int m_proc_size;
    int m_queue_size;

    int m_max_data_len;
};

}
