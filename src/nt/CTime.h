#pragma once

#include <sys/time.h>

namespace nt
{

class CTime
{
public:
	CTime()
	{
		gettimeofday(&m_tTimePrev, NULL);
	}

	~CTime(){}

	/**
		@return run time, in milli-second (ms)
	*/
	int getFuncRunTime()
	{
		struct timeval tNow;

		int septime = 0;

		gettimeofday( &tNow, NULL);
		septime = 1000000 * ( tNow.tv_sec - m_tTimePrev.tv_sec ) + tNow.tv_usec - m_tTimePrev.tv_usec;
		septime /= 1000;
		memmove( &m_tTimePrev, &tNow, sizeof(struct timeval) );

		return septime;
	}

private:
	timeval m_tTimePrev;
};

}// namespace nt

