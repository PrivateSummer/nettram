#pragma once
#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace nt
{

template<typename T>
class threadsafe_queue
{
private:
    mutable boost::mutex mut;
    std::queue<T> data_queue;
    boost::condition_variable data_cond;
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
        boost::lock_guard<boost::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one(); // 1
    }
    void wait_and_pop(T& value) // 2
    {
        boost::unique_lock<boost::mutex> lk(mut);
        data_cond.wait(lk, threadsafe_queue_wait_helper<std::queue<T> >(&data_queue));
        value = data_queue.front();
        data_queue.pop();
    }
    T wait_and_pop() // 3
    {
        boost::unique_lock<boost::mutex> lk(mut);
        data_cond.wait(lk, threadsafe_queue_wait_helper<std::queue<T> >(&data_queue)); // 4
        T res = data_queue.front();

        data_queue.pop();
        return res;
    }
    bool try_pop(T& value)
    {
        boost::lock_guard<boost::mutex> lk(mut);
        if(data_queue.empty())
            return false;

        value = data_queue.front();
        data_queue.pop();
        return true;
    }
    T try_pop()
    {
        boost::lock_guard<boost::mutex> lk(mut);
        if(data_queue.empty())
            return T(); // 5

        T res = data_queue.front();
        return res;
    }
    bool empty() const
    {
        boost::lock_guard<boost::mutex> lk(mut);
        return data_queue.empty();
    }
};

}
