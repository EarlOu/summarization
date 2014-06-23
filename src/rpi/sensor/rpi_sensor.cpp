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
#include <mutex>
#include <utility>

#include <opencv2/opencv.hpp>
using namespace cv;

typedef std::pair<int, int> Shot;

class FeatureReceiver {
public:
    FeatureReceiver(int sock_feature): _sock_feature(sock_feature) {}

    void start() {
        _thread = new std::thread(run_recv, _sock_feature, &_mutex, &_buf);
    }

    void readFeature(list<FeaturePacket>& features) {
        std::unique_lock<std::mutex> lock(_mutex);
        for (list<char*>::iterator it = _buf.begin(); it!=_buf.end(); it++) {
            features.push_back(FeaturePacket(*it));
            delete[] *it;
        }
        _buf.clear();
        lock.unlock();
    }

private:
    int _sock_feature;
    list<char*> _buf;
    std::mutex _mutex;
    std::thread* _thread;

    static void run_recv(int sock_feature, std::mutex* mutex, list<char*>* buf) {
        char* data = new char[INTER_FEATURE_SIZE];
        int n;
        while ((n = recvall(sock_feature, data, INTER_FEATURE_SIZE)) == INTER_FEATURE_SIZE) {
            char* b = new char[INTER_FEATURE_SIZE];
            memcpy(b, data, INTER_FEATURE_SIZE);
            std::unique_lock<std::mutex> lock(*mutex);
            buf->push_back(b);
            lock.unlock();
        }
        delete[] data;
        if (n != 0) {
            perror("Failed to receive feature");
            exit(EXIT_FAILURE);
        }
    }
};

class RpiSender: public Sender {
public:
    RpiSender(int sock_meta, int sock_feature, RpiVideoWriter* writer)
            :_sock_meta(sock_meta), _sock_feature(sock_feature), _skim_start(false),
             _writer(writer), _feature_buf(new char[INTER_FEATURE_SIZE]) {}

    virtual void sendFrame(InputArray frame, uint32_t time, int idx) {
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

    virtual void sendFeature(InputArray iFeature, float score, uint32_t time, int idx) {
        FeaturePacket p(iFeature.getMat(), score, time);
        p.pack(_feature_buf);
        if (sendall(_sock_feature, _feature_buf, INTER_FEATURE_SIZE) != INTER_FEATURE_SIZE) {
            perror("Failed to send feature");
            exit(EXIT_FAILURE);
        }
    }

    void finish() {
        if (_skim_start) {
            stream_shot(_skim_start_idx, _last_received_idx+1, _skim_start_time, _last_received_time);
        }
    }

private:
    int _sock_meta;
    int _sock_feature;
    bool _skim_start;
    int _skim_start_idx;
    int _last_received_idx;
    uint32_t _skim_start_time;
    uint32_t _last_received_time;
    RpiVideoWriter* _writer;

    char* _feature_buf;

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

static uint64_t gettime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void run_sensor(int width, int height, int encoder_fd, int sock_meta, int sock_feature,
        FeatureReceiver* receiver, bool* stop_flag) {

    RpiVideoCapture& cap = RpiVideoCapture::getInstance();
    cap.init(width, height, FPS);

    RpiVideoWriter& writer = RpiVideoWriter::getInstance();
    writer.init(width, height, 30, encoder_fd);

    RpiSender sender(sock_meta, sock_feature, &writer);
    Sensor sensor(&sender, false);

    list<FeaturePacket> features;
    Mat frame;
    int idx = 0;
    uint32_t prev_time;
    while (!(*stop_flag) && cap.read(frame)) {
        uint32_t time = gettime();
        if (idx != 0) {
            printf("Frame #%d, FPS = %f\n", idx, 1000.0f / (time-prev_time));
        }
        receiver->readFeature(features);
        sensor.next(idx, frame, gettime(), features);
        features.clear();
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
    char buf[STREAM_PACKET_SIZE];
    int n;
    while ((n = read(encode_fd, buf, STREAM_PACKET_SIZE)) > 0) {
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
    FeatureReceiver receiver(sock_feature);
    receiver.start();
    std::thread sensor_thread(run_sensor, width, height, encoder_pipe[1],
            sock_meta, sock_feature, &receiver, &stop_sensor_flag);
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
