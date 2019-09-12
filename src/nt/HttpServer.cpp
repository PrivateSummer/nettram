#include "HttpServer.h"

#ifdef _MSC_VER
#include <WinSock.h>
#include <process.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <netinet/tcp.h>
#include <regex.h>
#endif
#include <algorithm>
#include <stdexcept>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cstdio>
#include <cerrno>
#include "ScopedDestructor.h"
#include "ServerException.h"
#include "HttpHelper.h"
#include "HttpRespHelper.h"
#include "log.h"
#include "CTime.h"
#include "HttpStatus.h"
#include "SessionSystem.h"


#ifdef _MSC_VER
#define strcasecmp(str1, str2) _stricmp((str1), (str2))
#endif

namespace nt
{
	struct InterceptorData
	{
		std::vector<regex_t> regs;
		HandlerInterceptor *interceptor;
	};
	
	enum handle_return
	{
		H_NONE = 0,
		H_NOT_FOUND = 1,
		H_INTERCEPT = 2,
		H_WEBSOCKET = 3,
		H_ERROR = 4,
	};
}

using namespace nt;

HttpServer::HttpServer()
    : m_server_socket(-1), m_exit(false)
{
	m_max_listen_size = 256;
	m_sleep_micro_second = 1000;

	m_read_time_out = 3;
	m_write_time_out = 10;
}

HttpServer::~HttpServer()
{
	for(interceptors_type::iterator itr = m_interceptors.begin(); itr != m_interceptors.end(); ++itr)
	{
		for(size_t i = 0; i < (*itr)->regs.size(); ++i)
		{
			regfree(&(*itr)->regs[i]);
		}
		
		delete *itr;
	}
}

int HttpServer::init()
{
	return 0;
}

void HttpServer::stop()
{
    m_exit = true;
}

//������������
void HttpServer::do_http_options_work(HttpOptionType type, const void *value)
{
	switch(type)
	{
		case HTTP_OPT_IP:
		    {
				
		    }
		    break;
		case HTTP_OPT_PORT:
		    {
				m_conf.m_port = *(int*)value;
		    }
		    break;
		case HTTP_OPT_BUFF_SIZE:
		    {
				m_conf.m_buf_size = *(int*)value;
		    }
		    break;
	    case HTTP_OPT_LISTENQ:
		    {
				m_conf.m_listenq = *(int*)value;
		    }
		    break;
		case HTTP_OPT_QUEUE_SIZE:
		    {
				m_conf.m_queue_size = *(int*)value;
		    }
		    break;
		case HTTP_OPT_PROC_SIZE:
		    {
				m_conf.m_proc_size = *(int*)value;
		    }
		    break;
		case HTTP_OPT_MAX_DATA_LEN:
		    {
				m_conf.m_max_data_len = *(int*)value;
		    }
		    break;
		case HTTP_OPT_READ_TIMEOUT:
		    {
				m_read_time_out = *(int*)value;
		    }
		    break;
		case HTTP_OPT_WRITE_TIMEOUT:
		    {
				m_write_time_out = *(int*)value;
		    }
		    break;
		default:
		    break;
	};
}

// ������Ϣ����
void HttpServer::error_die(const char* sc)
{
	std::string err = std::string(sc) + ": " + std::string(strerror(errno));
	throw ServerException(err);
}

int HttpServer::startup(unsigned short port)
{
    int httpd = 0;
    struct sockaddr_in name;

    httpd = socket(PF_INET, SOCK_STREAM, 0);
    if (httpd == -1)
        error_die("socket");
	
	int opt = 1;
	if (setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
        error_die("setsockopt");
	}
	
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
        error_die("bind");

    if (listen(httpd, m_max_listen_size) < 0)
        error_die("listen");

    return(httpd);
}

void HttpServer::close_socket(int socket)
{
	info_log("close client{%d}\n", socket);
	close(socket);
}

int HttpServer::handle_request(nt::SharedHttpInput_t &request, nt::SharedHttpOutput_t &response)
{
	if( ! has_url(request->GetRequestURI()))
	{
		response->GetOutputImpl()->append("not found");
		response->SetHeader("Connection", "close");
		response->SetStatus(HTTP_NOT_FOUND);
		
		return H_NOT_FOUND;
	}
	
	const size_t nmatch = 1;
	regmatch_t pmatch[1];
	std::vector<HandlerInterceptor *> interceptors;
	for(interceptors_type::iterator itr = m_interceptors.begin(); itr != m_interceptors.end(); ++itr)
	{
		for(size_t i = 0; i < (*itr)->regs.size(); ++i)
		{
			int status = regexec(&(*itr)->regs[i], request->GetRequestURI().c_str(), nmatch, pmatch, 0); // ִ��������ʽ
			if (0 == status)
			{
				interceptors.push_back((*itr)->interceptor);
			}
		}
	}

    // ����������
	for(size_t i = 0; i < interceptors.size(); i++)
	{
		if( ! interceptors[i]->PreHandle(request.get(), response.get(), this))
		{
			return H_INTERCEPT;
		}
	}
	
	const std::string *upgrade = request->GetHeader("Upgrade");
	if(upgrade && strcasecmp(upgrade->c_str(), "websocket") == 0) //websocket
	{
		return H_WEBSOCKET;
	}

	std::string what;
	CTime serviceTime;
	try
	{
		service(request, response);		
	}
	catch(std::exception &ex)
	{
		response->SetStatus(HTTP_INTERNAL_SERVER_ERROR);
		err_log("cgi:%s, exception:%s", request->GetRequestURI().c_str(), ex.what());
		
		what = ex.what();
	}
		
	for(size_t i = 0; i < interceptors.size(); i++)
	{
		interceptors[i]->AfterCompletion(request.get(), response.get(), this, what.c_str());
	}
	debug_log("client ip:%s, uri:%s, service runtime:%d ms", request->GetRemoteAddr().c_str(), request->GetRequestURI().c_str(), serviceTime.getFuncRunTime());

	
	return H_NONE;
}

void HttpServer::accept_request(int client, nt::SharedHttpInput_t request)
{
    //�ں����˳�ʱ�Զ�ִ�� this->close_exit(client);
    ScopedDestructor<HttpServer, int> clientShutter(&HttpServer::close_socket, this, client);

	CTime ctime;

    info_log("accept_request socket:%d, ip:%s\n", client, request->GetRemoteAddr().c_str());

	nt::HttpHelper http(request);

	nt::SharedHttpOutput_t response = nt::CreateSharedHttpOutput();
	response->SetHeader("Content-Type", "text/html; charset=utf-8");
	response->SetHeader("Cache-Control", "no-cache");

	int ret = http.ReadData(client, m_conf.m_buf_size, m_conf.m_max_data_len);
	debug_log("uri:%s, queryString:%s", request->GetRequestURI().c_str(), request->GetQueryString().c_str());

	if (ret != 0)
	{
		if(request->GetMethod().empty())
		{
			response->SetHeader("Connection", "close");
			response->SetStatus(HTTP_OK);
		}
		else
		{
			response->SetStatus(ret > 0 ? ret : HTTP_INTERNAL_SERVER_ERROR);
		}
	}
	else
	{
		ret = handle_request(request, response);
		if(ret == H_WEBSOCKET)
		{
			int ret = m_webSocket.AddConnection(request, client);
			if(ret == 0)
			{
				clientShutter.Cancel(); //websocket������,��������ر�socket
				
				info_log("client ip:%s, uri:%s, websocket open runtime:%d ms", request->GetRemoteAddr().c_str(), 
						 request->GetRequestURI().c_str(), ctime.getFuncRunTime());
				return;
			}
			else
			{
				err_log("webSocket.AddConnection failed:%d", ret);
				response->SetStatus(HTTP_INTERNAL_SERVER_ERROR);
			}
		}
	}
	
	response->SetHeader("Content-Length", response->GetOutput().size());
	
	if( !request->GetSessionId().empty() && request->GetCookie("SESSIONID").empty())
	{
		response->SetHeader("Set-Cookie", std::string("SESSIONID=")+request->GetSessionId());
	}

	nt::HttpRespHelper respHelper(response);
	std::string pack;
	respHelper.CreateHeader(pack);

	//��Ϣͷ
	send(client, pack.c_str(), pack.size(), 0);
	//��Ϣ��
	send(client, response->GetOutput().c_str(), response->GetOutput().size(), 0);	

	info_log("client ip:%s, uri:%s, runtime:%d ms", request->GetRemoteAddr().c_str(), request->GetRequestURI().c_str(), ctime.getFuncRunTime());
}

void HttpServer::dowork()
{
    while(1)
    {
        struct sockaddr_in client_name;
        int client_name_len = sizeof(client_name);

        try
        {
            //����tcp����
            m_server_socket = startup((unsigned short)m_conf.m_port);
			printf("port:%d\n", m_conf.m_port);
            info_log("httpd running on port %d, socket %d\n", m_conf.m_port, m_server_socket);

            ScopedDestructor<HttpServer, int> socketShutter(&HttpServer::close_socket, this, m_server_socket);

            while (1)
            {
                if(m_exit)
                {
                    m_threadPool.stop();
                    return;
                }
                int client_sock = -1;

                client_sock = accept(m_server_socket,(struct sockaddr *)&client_name,(socklen_t*)&client_name_len);
                if (client_sock == -1)
                {
                    error_die("accept");
                }

                struct timeval read_timeout={ m_read_time_out, 0};
				struct timeval write_timeout={ m_write_time_out, 0};
				//���÷��ͳ�ʱ
                int snd_ret = setsockopt(client_sock,SOL_SOCKET,SO_SNDTIMEO,(const char*)&write_timeout,sizeof(write_timeout));
				//���ý��ճ�ʱ
                int rcv_ret = setsockopt(client_sock,SOL_SOCKET,SO_RCVTIMEO,(const char*)&read_timeout,sizeof(read_timeout));
				int tcp_no_delay = 1;
				setsockopt( client_sock, IPPROTO_TCP, TCP_NODELAY, (void *)&tcp_no_delay, sizeof(tcp_no_delay));
				int flag = 1;
				int ret = setsockopt( client_sock, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag) );
				if (ret < 0) 
				{
					err_log("Couldn't setsockopt(TCP_NODELAY)");
				}
                if(snd_ret < 0 || rcv_ret < 0)
                {
                    close_socket(client_sock);
                    error_die("setsockopt");
                }

				nt::SharedHttpInput_t request = nt::CreateSharedHttpInput();

				request->SetRemoteAddr(inet_ntoa(client_name.sin_addr));
				request->SetRemotePort(client_name.sin_port);

                //�̳߳�
                m_threadPool.run(boost::bind(&HttpServer::accept_request, this, client_sock, request));
            }
        }
        catch(std::exception &ex)
        {
			err_log("%s. exception exit!!!", ex.what());
			printf("%s. exception exit!!!\n", ex.what());

			exit(0);
        }

        usleep(m_sleep_micro_second);
    }
    return;
}


int HttpServer::start()
{
    try
    {
		if(init() != 0)
		{
			return -1;
		}
    }
    catch(std::exception &ex)
    {
        err_log("%s", ex.what());
		return -1;
    }
	m_webSocket.Start();
	
    m_threadPool.setMaxQueueSize(m_conf.m_queue_size);
	m_threadPool.start(m_conf.m_proc_size);

    m_exit = false;

	dowork();
	return 0;
}

void HttpServer::put_interceptor(const std::vector<std::string> &path, HandlerInterceptor *interceptor)
{
	InterceptorData *data = new InterceptorData();

	for(std::vector<std::string>::const_iterator pathItr = path.begin(); pathItr != path.end(); ++pathItr)
	{
		debug_log("path:%s", pathItr->c_str());
		regex_t reg;
	    regcomp(&reg, pathItr->c_str(), REG_EXTENDED);//��������
		data->regs.push_back(reg);
	}
	
	data->interceptor = interceptor;
	m_interceptors.push_back(data);
}

void HttpServer::add_url(const std::string &url)
{
	m_url.insert(url);
}

bool HttpServer::has_url(const std::string &url) const
{
	return m_url.find(url) != m_url.end();
}
