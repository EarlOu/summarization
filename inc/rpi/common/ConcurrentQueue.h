#ifndef CONCURRENT_QUEUE_H
#define CONCURRENT_QUEUE_H

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template<typename T>
class ConcurrentQueue {
public:
    T pop() {
        std::unique_lock<std::mutex> mlock(_mutex);
        while (_queue.empty()) {
            _cond.wait(mlock);
        }
        T& item = _queue.front();
        _queue.pop();
        return item;
    }
    void push(T& item) {
        std::unique_lock<std::mutex> mlock(_mutex);
        _queue.push(item);
        mlock.unlock();
        _cond.notify_one();
    }
private:
    std::queue<T> _queue;
    std::mutex _mutex;
    std::condition_variable _cond;
};
#endif
