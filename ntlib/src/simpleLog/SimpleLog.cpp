#include "SimpleLog.h"

#include <string>
#include <stdio.h>
#include <stdarg.h>
#include "SimpleLogImpl.h"

namespace nt
{

namespace simpleLog
{

Log::Log(const std::string & name, int mode)
{
	m_logImpl = new SimpleLogImpl();
	
	m_logImpl->m_logArgs.mode = mode;
	m_logImpl->m_logArgs.name = name;
}

Log::~Log()
{
	delete m_logImpl;
}

int Log::start()
{
	int ret = 2;
	ret = m_logImpl->start();
	
	return ret;
}

void Log::setRoll(int roll)
{
	m_logImpl->setRoll(roll);
}

int Log::getRoll() const 
{
    return m_logImpl->getRoll();
}

void Log::setLogLevel(int level)
{
	m_logImpl->setLogLevel(level);
}

int Log::getLogLevel() const 
{
    return m_logImpl->getLogLevel();
}

void Log::setMultithread(int m)
{
	m_logImpl->setMultithread(m);
}

int Log::getMultithread() const
{
	return m_logImpl->getMultithread();
}

const std::string & Log::getLogFileName() const
{
	return m_logImpl->getLogFileName();
}

void Log::print_log(int log_level, const char *fmt, ...)
{
	if(m_logImpl->checkLogLevel(log_level) != 0)
    {
        return;
    }

    va_list ap;
    va_start (ap, fmt);
	m_logImpl->print_log(log_level, fmt, ap);
    va_end (ap);
}

}; //namespace simpleLog

} //namespace nt
