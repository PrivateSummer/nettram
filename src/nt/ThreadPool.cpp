/*
 * ThreadPool.cpp
 *
 *  Created on: Feb 28, 2018
 *      Author: hlb
 */

#include "ThreadPool.h"
#include <cassert>

using namespace std;
#ifdef WITH_BOOST
using namespace boost;
#endif
ThreadPool::ThreadPool(const string &name):
    _name(name),
    _maxQueueSize(0),
    _running(false)
{

}

ThreadPool::~ThreadPool()
{
    if(_running)
    {
        stop();
    }
}

void ThreadPool::start(int numThreads)
{
    if(_threads.size() != 0)
    {
        return;
    }
    _running = true;

    for(int i = 0; i < numThreads; ++i)
    {
#ifdef WITH_BOOST
        _threads.create_thread(boost::bind(&ThreadPool::runInThread, this));
#else
        _threads.push_back(std::thread(std::bind(&ThreadPool::runInThread, this)));
#endif
    }
}

void ThreadPool::stop()
{
    {
        unique_lock<mutex>  lock(_mutex);
        _running = false;
        _notEmpty.notify_all();
    }

#ifdef WITH_BOOST
    _threads.join_all();
#else
    for(int i = 0; i < numThreads; ++i)
    {
        if (_threads[i].joinable())
        {
            _threads[i].join();
        }
    }
#endif
}

void ThreadPool::run(const Task &f)
{
    if(_threads.size() == 0)
    {
        f();
    }
    else
    {
        unique_lock<mutex> lock(_mutex);
        while(isFull())
        {
            _notFull.wait(lock);
        }

        assert(!isFull());
        _queue.push_back(f);
        _notEmpty.notify_one();
    }
}

ThreadPool::Task ThreadPool::take()
{
    unique_lock<mutex> lock(_mutex);

    while(_queue.empty() && _running)
    {
        _notEmpty.wait(lock);
    }

    Task task;
    if(!_queue.empty())
    {
        task = _queue.front();
        _queue.pop_front();

        if(_maxQueueSize > 0)
        {
            _notFull.notify_one();
        }
    }
    return task;
}

bool ThreadPool::isFull()
{
    return _maxQueueSize > 0 && _queue.size() >= _maxQueueSize;
}

void ThreadPool::runInThread()
{
    try
    {
        while(_running)
        {
            Task task = take();
            if(task)
            {
                task();
            }
        }
    }
    catch (const std::exception &ex)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", _name.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        //abort();
    }
    catch(...)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", _name.c_str());
    }
}
