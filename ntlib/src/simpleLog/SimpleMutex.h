#pragma once

#ifdef _MSC_VER
#pragma once
#include <process.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

namespace nt
{

/*
* by Summer
* 2017-03-12
* header-file only
*/
class ThreadMutex
{
private:  // noncopyable
    ThreadMutex( const ThreadMutex & );
    ThreadMutex &operator=( const ThreadMutex & );

public:
    ThreadMutex()
    {
#ifdef _MSC_VER
        m_threadParameter = CreateMutex(NULL, FALSE, NULL);
#else
        pthread_mutex_init(&m_mutex, NULL);
#endif
    }
    ~ThreadMutex()
    {
#ifdef _MSC_VER
        CloseHandle(m_threadParameter);
#else
        pthread_mutex_destroy( &m_mutex );
#endif
    }
    void lock()
    {
#ifdef _MSC_VER
        WaitForSingleObject(m_threadParameter, INFINITE);
#else
        pthread_mutex_lock(&m_mutex);
#endif
    }
    void unlock()
    {
#ifdef _MSC_VER
        ReleaseMutex(m_threadParameter);
#else
        pthread_mutex_unlock(&m_mutex);
#endif
    }

private:
#ifdef _MSC_VER
    HANDLE  m_threadParameter;
#else
    pthread_mutex_t m_mutex;
#endif
};

template <typename MutexType>
class ScopedLock
{
private:  // noncopyable
    ScopedLock( const ScopedLock & );
    ScopedLock &operator=( const ScopedLock & );

public:
    ScopedLock(MutexType &threadlock)
        : m_mutex(&threadlock), isLocked(false)
    {
        lock();
    }

    ~ScopedLock()
    {
        if (ownsLock())
        {
            m_mutex->unlock();
        }
    }

    void lock()
    {
        if (m_mutex == 0)
        {
            return;
        }
        if (ownsLock())
        {
            return;
        }
        m_mutex->lock();
        isLocked = true;
    }

    void unlock()
    {
        if (m_mutex == 0)
        {
            return;
        }
        if (!ownsLock())
        {
            return;
        }
        m_mutex->unlock();
        isLocked = false;
    }
protected:
    bool ownsLock() const
    {
        return isLocked;
    }
private:
    MutexType *m_mutex;
    bool isLocked;
};

} //namespace nt
