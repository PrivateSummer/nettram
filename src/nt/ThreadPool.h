#pragma once

#include <string>
#include <deque>
#include <vector>
#include <memory>
#ifdef WITH_BOOST
#include <boost/thread.hpp>
#include <boost/functional.hpp>
#else
#include <thread>
#include <functional>
#endif
#include "noncopyable.h"

class ThreadPool : public nt::noncopyable
{
public:
#ifdef WITH_BOOST
    typedef boost::function<void()> Task;
#else
	typedef std::function<void()> Task;	
#endif
    explicit ThreadPool(const std::string &name = std::string());
    ~ThreadPool();

    void start(int numThreads);//设置线程数，创建numThreads个线程
    void stop();//线程池结束
    void run(const Task& f);//任务f在线程池中运行
    void setMaxQueueSize(int maxSize) { _maxQueueSize = maxSize; }//设置任务队列可存放最大任务数
    bool isFull();//任务队列是否已满

private:
    void runInThread();//线程池中每个thread执行的function
    Task take();//从任务队列中取出一个任务

#ifdef WITH_BOOST
    boost::mutex _mutex;
    boost::condition_variable _notEmpty;
    boost::condition_variable _notFull;
	boost::thread_group _threads;
#else
	std::mutex _mutex;
    std::condition_variable _notEmpty;
    std::condition_variable _notFull;
	std::vector<std::thread> _threads;	
#endif
    std::string _name;
    std::deque<Task> _queue;
    size_t _maxQueueSize;
	volatile bool _running;
};
