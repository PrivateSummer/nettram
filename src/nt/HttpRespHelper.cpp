#include "HttpRespHelper.h"
#include <sys/socket.h>
#include <errno.h>
#include "log.h"

namespace nt
{

HttpRespHelper::HttpRespHelper(SharedHttpOutput_t output)
{
	m_output = output;
}

HttpRespHelper::~HttpRespHelper()
{}

struct CodeToInfo
{
	int code;
	const char* info;
};

void HttpRespHelper::CreateHeader(std::string &header)
{
	CodeToInfo codeToInfo[] =
	{
		{HTTP_SWITCHING_PROTOCOLS, "Switching Protocols"},
		{HTTP_OK, "OK"},
		{HTTP_FOUND, "Found"},
		{HTTP_BAD_REQUEST, "Bad Request"},
		{HTTP_FORBIDDEN, "Forbidden"},
		{HTTP_UNAUTHORIZED, "Unauthorized"},
		{HTTP_NOT_FOUND, "Not Found"},
		{HTTP_PAYLOAD_TOO_LARGE, "Payload Too Large"},
		{HTTP_URI_TOO_LONG, "URI Too Long"},
		{HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE, "Request Header Fields Too Large"},
		{HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error"},
	    {HTTP_METHOD_NOT_IMPLEMENTED, "Not Implemented"},
		{0, NULL}
	};

	const char *info = "OK";

	for (int i = 0; ; ++i)
	{
		CodeToInfo* iter = &(codeToInfo[i]);
		if (NULL == iter->info) break;

		if (m_output->GetStatus() == iter->code)
		{
			info = iter->info;
		}
	}

	char buf[1024];

	int n = snprintf(buf, sizeof(buf) - 1, "HTTP/%s %d %s\r\n", m_output->GetVersion().c_str(), m_output->GetStatus(), info);
	buf[n] = 0;

	header += buf;

	for (size_t i = 0; i < m_output->GetHeaders().size(); ++i)
	{
		int n = snprintf(buf, sizeof(buf) - 1, "%s: %s\r\n", m_output->GetHeaders()[i].key.c_str(), m_output->GetHeaders()[i].value.c_str());
		buf[n] = 0;
		header += buf;
	}
	header += "\r\n";
}


}// namespace nt
