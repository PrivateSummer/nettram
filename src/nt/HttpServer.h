#pragma once

#include <map>
#include <string>
#include <list>
#include <sstream>

#include "ThreadPool.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "HttpConfig.h"
#include "WebSocket.h"
#include "HandlerInterceptor.h"

namespace nt
{
	enum HttpOptionType
	{
		HTTP_OPT_IP = 1,
		HTTP_OPT_PORT = 2,
		HTTP_OPT_BUFF_SIZE = 3,
	    HTTP_OPT_LISTENQ = 4,
		HTTP_OPT_QUEUE_SIZE = 5,
		HTTP_OPT_PROC_SIZE = 6,
		HTTP_OPT_MAX_DATA_LEN = 7,
		HTTP_OPT_READ_TIMEOUT = 8,
		HTTP_OPT_WRITE_TIMEOUT = 9,
	};
	
	struct InterceptorData;
	
	class HttpServer
	{
	public:
		HttpServer();
		virtual ~HttpServer();

		void stop();
		int start();
		
		virtual void service(nt::SharedHttpInput_t request, nt::SharedHttpOutput_t response) = 0;
		
		template<typename T>
		void http_options(HttpOptionType type, const T &value)
		{
			do_http_options_work(type, &value);
		}
		
		void put_interceptor(const std::vector<std::string> &path, HandlerInterceptor *interceptor);
		
		void add_url(const std::string &url);
		bool has_url(const std::string &url) const;
	protected:
	    virtual int init();

	private:
		void dowork();
		void accept_request(int client, nt::SharedHttpInput_t request_project);
		int handle_request(nt::SharedHttpInput_t &request, nt::SharedHttpOutput_t &response);
		
		int startup(unsigned short port);

		void close_socket(int socket);

		void error_die(const char* sc);
		
		void do_http_options_work(HttpOptionType type, const void *value);
	private:
	    nt::HttpConfig m_conf;
		long m_read_time_out;
		long m_write_time_out;
		int m_server_socket;
		bool m_exit;

		int m_max_listen_size;
		int m_sleep_micro_second;

		ThreadPool m_threadPool;
		
		WebSocket m_webSocket;
		
		
		typedef std::vector<InterceptorData *> interceptors_type;
		
		interceptors_type m_interceptors;
		
		std::set<std::string> m_url;
	}; // HttpServer

}// namespace nt
