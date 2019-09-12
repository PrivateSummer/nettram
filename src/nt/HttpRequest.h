#pragma once

#include <vector>
#include <string>
#include <string.h>
#include <sstream>
#include "use_shared_ptr.h"
#include "HttpSession.h"
#include "SessionSystem.h"
#include "str.h"

namespace nt
{
	
enum HTTP_METHOD
{
	UNKNOW = 0,
	GET = 1,
	POST = 2
};

class HttpRequest
{
public:
	HttpRequest()
	{
		m_methodId = UNKNOW;
		m_contentLength = 0;
		m_remotePort = 0;
	    m_keepAlive = 0;
		m_version = "1.0";
		m_session = NULL;
	}
    virtual ~HttpRequest()
	{
		if(m_session != NULL)
		{
			delete m_session;
			m_session = NULL;
		}
	}


	struct header
	{
		std::string key;
		std::string value;
	};

	template<typename T>
	void SetHeader(const std::string& key, const T& value)
	{
		std::stringstream ss;
		ss << value;
		header h = { key, ss.str() };

		m_headers.push_back(h);
	}
	
	const std::string * GetHeader(const std::string& key) const
	{
		for (size_t i = 0; i < m_headers.size(); i++)
		{
			if (strcasecmp(m_headers[i].key.c_str(), key.c_str()) == 0)
			{
				return &m_headers[i].value;
			}
		}
		return NULL;
	}

	const std::vector<HttpRequest::header>& GetHeaders() const
	{
		return m_headers;
	}

	std::vector<HttpRequest::header>& Headers()
	{
		return m_headers;
	}

	void ClearHeaders()
	{
		m_headers.clear();
	}

	void SetMethodId(HTTP_METHOD methodId)
	{
		m_methodId = methodId;
	}

	HTTP_METHOD GetMethodId() const
	{
		return m_methodId;
	}

	void SetMethod(const std::string& method)
	{
		m_method = method;
	}

	const std::string& GetMethod() const
	{
		return m_method;
	}

	std::string& Method()
	{
		return m_method;
	}

	void SetRequestURI(const std::string &uri)
	{
		m_requestURI = uri;
	}

	const std::string& GetRequestURI() const
	{
		return m_requestURI;
	}

	std::string& RequestURI()
	{
		return m_requestURI;
	}

	void SetInput(const std::string& input)
	{
		m_input = input;
	}

	const std::string & GetInput() const
	{
		return m_input;
	}

	std::string * GetInputImpl()
	{
		return &m_input;
	}

	void SetRemoteAddr(const std::string& addr)
	{
		m_remoteAddr = addr;
	}

	const std::string& GetRemoteAddr() const
	{
		return m_remoteAddr;
	}

	void SetQueryString(const std::string& queryString)
	{
		m_queryString = queryString;
	}

	const std::string& GetQueryString() const
	{
		return m_queryString;
	}

	void SetRemotePort(int remotePort)
	{
		m_remotePort = remotePort;
	}

	int GetRemotePort() const
	{
		return m_remotePort;
	}

	void SetKeepAlive(int keepAlive)
	{
		m_keepAlive = keepAlive;
	}

	int GetKeepAlive() const
	{
		return m_keepAlive;
	}

	void SetContentLength(int contentLength)
	{
		m_contentLength = contentLength;
	}

	int GetContentLength() const
	{
		return m_contentLength;
	}
	
	void SetVersion(const std::string& version)
	{
		m_version = version;
	}
	
	const std::string & GetVersion() const
	{
		return m_version;
	}
	
	const std::string & GetSessionId() const
	{
		if(m_session)
		{
		    return m_session->GetSessionId();
		}
		static std::string default_empty_str = "";
		return default_empty_str;
	}
	
	HttpSession *GetSession(bool create = false)
	{
		if(m_session == NULL)
		{
			std::string sessionId;
			const std::string *cookie = GetHeader("Cookie");
			if(cookie != 0)
			{
				std::vector<std::string> kv = split(*cookie, ':');
				for(size_t i = 0; i < kv.size(); i++)
				{
					std::vector<std::string> v = split(kv[i], '=');
					if(v.size() == 2)
					{
						trim(v[0]);
						if(v[0] == "SESSIONID")
						{
						    sessionId = v[1];
							trim(sessionId);
						}
					}
				}
			}
			if(sessionId.size() > 0 && ! SessionIdValid(sessionId))
			{
				return NULL;
			}
			
			if(create)
			{
				m_session = SessionSystem::Instance()->MutableGet(sessionId);
			}
			else
			{
				m_session = SessionSystem::Instance()->Get(sessionId);
			}
		}

		return m_session;
	}
	std::string GetCookie(const std::string& key)
	{
		const std::string *cookies = GetHeader("Cookie");
		if(cookies != 0)
		{
			std::vector<std::string> kv = split(*cookies, ':');
			for(size_t i = 0; i < kv.size(); i++)
			{
				std::vector<std::string> v = split(kv[i], '=');
				if(v.size() == 2 && v[0] == key)
				{
					return v[1];
				}
			}
		}
		return "";
	}
private:
	std::vector<HttpRequest::header> m_headers;
	int m_contentLength;
	HTTP_METHOD m_methodId;
	std::string m_requestURI;
	std::string m_input;
	std::string m_remoteAddr;
	int m_remotePort;
	std::string m_queryString;
	std::string m_method;
	int m_keepAlive;
	std::string m_version;
	HttpSession *m_session;
};

#ifdef WITH_BOOST
typedef boost::shared_ptr<nt::HttpRequest> SharedHttpInput_t;
#else 
typedef std::shared_ptr<nt::HttpRequest> SharedHttpInput_t;
#endif
#define CreateSharedHttpInput() make_shared<nt::HttpRequest>(); 

}
