#pragma once

#include <string>
#include <stdio.h>

namespace nt
{

namespace simpleLog
{
	enum 
	{
		LOG_LELEVE_FATAL = 0,
		LOG_LELEVE_ERROR = 1,
		LOG_LELEVE_WARNING = 2,
		LOG_LELEVE_INFO = 3,
		LOG_LELEVE_DEBUG = 4,
		TOTLE_LOG_LELEVE_SIZE = 5
	};
	
	enum ConstructorMode
	{
		SET_NAME = 0,
		READ_CONFIG = 1
	};

	class SimpleLogImpl;
	
	class Log
	{
	public:
		Log(const std::string & name, int mode = READ_CONFIG);
		~Log();
		int start();

		void print_log(int log_level, const char *fmt, ...)
		__attribute__((format(printf, 3, 4)));
		const std::string & getLogFileName() const;
		
    public:
	    /* 滚动文件名. 默认 ROLL_DAILY (1) */
        void setRoll(int roll);
		int getRoll() const;
		
		/* 日志级别. 默认 LOG_LELEVE_INFO (3) */
		void setLogLevel(int level);
		int getLogLevel() const;
		
		/* 多线程支持. 1支持, 0不支持. 默认 0 */
		void setMultithread(int m);
		int getMultithread() const;
	private:
        Log(const Log &);
		Log & operator=(const Log &);
		
        SimpleLogImpl *m_logImpl;
	};

}; //namespace simpleLog

}//namespace nt
