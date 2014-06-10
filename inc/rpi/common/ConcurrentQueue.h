#ifndef CONCURRENT_QUEUE_H
#define CONCURRENT_QUEUE_H

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template<typename T>
class ConcurrentQueue {
public:
    T pop();
    void push(T);
private:
    std::queue<T> _queue;
    std::mutex _mutex;
    std::condition_variable _cond;
};
#endif
