#pragma once

#include <vector>
#include <string>
#include <string.h>
#include <sstream>
#include "use_shared_ptr.h"

namespace nt
{
class LogicInput
{
public:
	LogicInput()
	{
		m_contentLength = 0;
		m_remotePort = 0;
	    m_keepAlive = 0;
		m_code = 200;
		m_version = "1.0";
	}
    virtual ~LogicInput(){};


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

	const std::vector<LogicInput::header>& GetHeaders() const
	{
		return m_headers;
	}

	std::vector<LogicInput::header>& Headers()
	{
		return m_headers;
	}

	void ClearHeaders()
	{
		m_headers.clear();
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

	std::string& QueryString()
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

	void SetCode(int code)
	{
		m_code = code;
	}

	int GetCode() const
	{
		return m_code;
	}
	
	void SetVersion(const std::string& version)
	{
		m_version = version;
	}
	
	std::string GetVersion() const
	{
		return m_version;
	}
private:
	std::vector<LogicInput::header> m_headers;
	int m_contentLength;
	std::string m_requestURI;
	std::string m_input;
	std::string m_remoteAddr;
	int m_remotePort;
	std::string m_queryString;
	std::string m_method;
	int m_keepAlive;
	int m_code;
	std::string m_version;
};

#ifdef WITH_BOOST
typedef boost::shared_ptr<nt::LogicInput> SharedLogicInput_t;
#else 
typedef std::shared_ptr<nt::LogicInput> SharedLogicInput_t;
#endif
#define CreateSharedLogicInput() make_shared<nt::LogicInput>(); 

}
