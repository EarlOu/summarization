#include "gmm/Sensor.h"
#include "gmm/Sender.h"
#include "rpi/common/Common.h"
#include "rpi/common/NetUtil.h"
#include "rpi/common/ConcurrentQueue.h"
#include "rpi/rpi_opencv/RpiVideoCapture.h"
#include "rpi/rpi_opencv/RpiVideoWriter.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <thread>
#include <utility>

#include <opencv2/opencv.hpp>
using namespace cv;

typedef std::pair<int, int> Shot;

class RpiSender: public Sender {
public:
    RpiSender() {}

    virtual void sendFrame(InputArray frame, size_t time, int idx) {}

    virtual void sendFeature(InputArray iFeature, double score, size_t time, int idx) {}
private:
};

static size_t gettime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// class ShotInfoSender {
// public:
//     ShotInfoSender(int sockfd): _sockfd(sockfd), _thread(run, sockfd, &_queue) {}
//
//     void sendShotInfo(Shot& s) {
//         _queue.push(s);
//     }
//     void end() {
//         _queue.push(Shot(0, 0));
//         _thread.join();
//     }
// private:
//     ConcurrentQueue<Shot> _queue;
//     int _sockfd;
//     std::thread _thread;
//
//     static void run(int fd, ConcurrentQueue<Shot>* q) {
//         Shot s;
//         int32_t buf[2];
//         while (true) {
//             s = q->pop();
//             if (s.first == 0 && s.second == 0) break;
//             buf[0] = s.first;
//             buf[1] = s.second;
//             if (sendall(fd, (char*) buf, 2 * sizeof(int32_t)) < 0) {
//                 perror("Failed to send segment information");
//                 exit(EXIT_FAILURE);
//             }
//         }
//     }
// };

void run_sensor(RpiVideoCapture* cap, RpiVideoWriter* writer, bool* stop_flag) {
    RpiSender sender;
    Sensor sensor(&sender, false);

    list<ReceivedFeature> features;
    Mat frame;
    int idx = 0;
    while (!(*stop_flag) && cap->read(frame)) {
        sensor.next(idx, frame, gettime(), features);
        idx++;
    }
    sensor.finish();
    cap->release();
    writer->release();
}

char recv_msg(int msg_fd) {
    char msg;
    int n;
    if ((n = recv(msg_fd, &msg, 1, 0)) <=0) {
        if (n == 0) {
            printf("Connction Break");
        } else {
            perror("Failed to recv msg");
        }
        exit(EXIT_FAILURE);
    }
    return msg;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s <server ip>\n", argv[0]);
        return -1;
    }

    const int width = 320;
    const int height = 240;

    RpiVideoCapture& cap = RpiVideoCapture::getInstance();
    cap.init(width, height);

    int encoder_pipe[2];
    if (pipe(encoder_pipe) < 0) {
        perror("Failed to create encoder pipe");
        exit(EXIT_FAILURE);
    }
    RpiVideoWriter& writer = RpiVideoWriter::getInstance();
    writer.init(width, height, 30, encoder_pipe[1]);

    sockaddr_in addr;
    if (inet_aton(argv[1], (struct in_addr*) &addr) < 0) {
        printf("Incorrect IP address: %s\n", argv[1]);
        return -1;
    }

    int fd_msg = connect_to(addr.sin_addr.s_addr, PORT);
    printf("Successfully connect to server, fd=%d\n", fd_msg);

    int fd_feature = listen_single_connect(PORT_FEATURE);
    printf("Successfully create feature channel, fd=%d\n", fd_feature);

    int fd_video = listen_single_connect(PORT_VIDEO);
    printf("Successfully create video channel, fd=%d\n", fd_video);

    char msg = recv_msg(fd_msg);
    assert(msg == MSG_START);

    bool stop_sensor_flag = false;
    std::thread sensor_thread(run_sensor, &cap, &writer, &stop_sensor_flag);

    msg = recv_msg(fd_msg);
    assert(msg == MSG_STOP);

    stop_sensor_flag = true;
    sensor_thread.join();
}
