#pragma once

#ifdef WITH_BOOST
#include "threadsafe_queue_use_boost.h"
#else
#include <queue>
#include <mutex>
#include <thread>

namespace nt
{

template<typename T>
class threadsafe_queue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    threadsafe_queue()
    {}

    template<typename T1>
    class threadsafe_queue_wait_helper
    {
        T1 *data_queue;
    public:
        threadsafe_queue_wait_helper(T1 *queue) : data_queue(queue)
        {}
        bool operator()()
        {
            return !data_queue->empty();
        }
    };

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one(); // 1
    }
    void wait_and_pop(T &value) // 2
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, threadsafe_queue_wait_helper<std::queue<T> >(&data_queue));
        value = data_queue.front();
        data_queue.pop();
    }
    T wait_and_pop() // 3
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, threadsafe_queue_wait_helper<std::queue<T> >(&data_queue)); // 4
        T res = data_queue.front();

        data_queue.pop();
        return res;
    }
    bool try_pop(T &value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return false;

        value = data_queue.front();
        data_queue.pop();
        return true;
    }
    T try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return T(); // 5

        T res = data_queue.front();
        return res;
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};

}
#endif
