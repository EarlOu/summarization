#ifndef THREAD_SENDER_H
#define THREAD_SENDER_H

#include "rpi/common/ConcurrentQueue.h"
#include <thread>

class ThreadSender {
public:

    void send(char* buf, int len);
private:
    typedef std::pair<char*, len> Packet;
    std::thread* _thread;
    ConcurrentQueue<Packet> _queue;

    static void run(int sockfd, ConcurrentQueue<Packet>* queue) {
        Packet p;
        while (true) {
            p = queue->pop();
            if (!p.first) break;
            if (sendall(sockfd, ))
        }
    }
};

#endif
