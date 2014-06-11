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
    RpiSender(int sock_meta, RpiVideoWriter* writer)
            :_sock_meta(sock_meta), _skim_start(false), _writer(writer) {}

    virtual void sendFrame(InputArray frame, size_t time, int idx) {
        _writer->write(frame);
        assert(idx > _last_received_idx);
        if (_skim_start) {
            if (idx != _last_received_idx + 1) { // the shot is break
                stream_shot(_skim_start_idx, _last_received_idx+1, _skim_start_time, _last_received_time);
                _skim_start_idx = idx;
                _skim_start_time = time;
            }
        } else {
            _skim_start = true;
            _skim_start_idx = idx;
            _skim_start_time = time;
        }
        _last_received_idx = idx;
        _last_received_time = time;
    }

    virtual void sendFeature(InputArray iFeature, double score, size_t time, int idx) {}

    void finish() {
        if (_skim_start) {
            stream_shot(_skim_start_idx, _last_received_idx+1, _skim_start_time, _last_received_time);
        }
    }

private:
    int _sock_meta;
    bool _skim_start;
    int _skim_start_idx;
    int _last_received_idx;
    uint32_t _skim_start_time;
    uint32_t _last_received_time;
    RpiVideoWriter* _writer;

    void stream_shot(int start_idx, int stop_idx, uint32_t start_time, uint32_t stop_time) {
        printf("Streaming Shot: %d %d %u %u\n", _skim_start_idx, _last_received_idx + 1
                , start_time, stop_time);
        uint32_t buf[3];
        buf[0] = start_time;
        buf[1] = stop_time;
        buf[2] = stop_idx - start_idx;
        if (sendall(_sock_meta, (char*) buf, 3 * sizeof(uint32_t)) != 3 * sizeof(uint32_t)) {
            perror("Failed to send shot info");
            exit(EXIT_FAILURE);
        }
    }
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

void run_sensor(int width, int height, int encoder_fd, int sock_meta, bool* stop_flag) {

    RpiVideoCapture& cap = RpiVideoCapture::getInstance();
    cap.init(width, height);

    RpiVideoWriter& writer = RpiVideoWriter::getInstance();
    writer.init(width, height, 30, encoder_fd);

    RpiSender sender(sock_meta, &writer);
    Sensor sensor(&sender, false);

    list<ReceivedFeature> features;
    Mat frame;
    int idx = 0;
    uint32_t prev_time;
    while (!(*stop_flag) && cap.read(frame)) {
        uint32_t time = gettime();
        if (idx != 0) {
            printf("Frame #%d, FPS = %f\n", idx, 1000.0f / (time-prev_time));
        }
        sensor.next(idx, frame, gettime(), features);
        idx++;
        prev_time = time;
    }
    sensor.finish();
    sender.finish();
    cap.release();
    writer.release();
    close(encoder_fd);
}

void run_video(int sockfd, int encode_fd) {
    char buf[65536];
    int n;
    while ((n = read(encode_fd, buf, 65536)) > 0) {
        if (sendall(sockfd, buf, n) != n) {
            perror("Failed to stream video");
            exit(EXIT_FAILURE);
        }
    }
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

    int encoder_pipe[2];
    if (pipe(encoder_pipe) < 0) {
        perror("Failed to create encoder pipe");
        exit(EXIT_FAILURE);
    }

    sockaddr_in addr;
    if (inet_aton(argv[1], &addr.sin_addr) < 0) {
        printf("Incorrect IP address: %s\n", argv[1]);
        return -1;
    }

    printf("Trying to connect to server...\n");
    int sock_feature_listen = listen_connect(PORT_FEATURE);
    int sock_video_listen = listen_connect(PORT_VIDEO);
    int sock_meta = connect_to(addr.sin_addr.s_addr, PORT);
    printf("Successfully connect to server, fd=%d\n", sock_meta);

    int sock_feature = accept_connect(sock_feature_listen);
    printf("Successfully create feature channel, fd=%d\n", sock_feature);
    close(sock_feature_listen);

    int sock_video = accept_connect(sock_video_listen);
    printf("Successfully create video channel, fd=%d\n", sock_video);
    close(sock_video_listen);

    char msg = recv_msg(sock_meta);
    assert(msg == MSG_START);

    printf("Starting...\n");
    bool stop_sensor_flag = false;
    std::thread sensor_thread(run_sensor, width, height, encoder_pipe[1], sock_meta, &stop_sensor_flag);
    std::thread video_thread(run_video, sock_video, encoder_pipe[0]);

    msg = recv_msg(sock_meta);
    assert(msg == MSG_STOP);
    printf("Stopping...\n");

    stop_sensor_flag = true;
    sensor_thread.join();
    video_thread.join();

    close(sock_meta);
    close(sock_video);
    close(sock_feature);
}
