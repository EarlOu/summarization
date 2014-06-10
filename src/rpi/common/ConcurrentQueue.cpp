#include "rpi/common/ConcurrentQueue.h"

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template<typename T>
T ConcurrentQueue<T>::pop() {
    std::unique_lock<std::mutex> mlock(_mutex);
    while (_queue.empty()) {
        _cond.wait(mlock);
    }
    T& item = _queue.front();
    _queue.pop();
    return item;
}

template<typename T>
void ConcurrentQueue<T>::push(T item) {
    std::unique_lock<std::mutex> mlock(_mutex);
    _queue.push(item);
    mlock.unlock();
    _cond.notify_one();
}
