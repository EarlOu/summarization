#include "gmm/Common.h"
#include "rpi/common/Common.h"
#include "rpi/common/ConcurrentQueue.h"
#include "rpi/common/NetUtil.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <thread>
#include <vector>
using std::vector;

#include <opencv2/opencv.hpp>
using namespace cv;

const int MAX_N_SENSOR = 10;

void wait_enter() {
    static char buf[1024];
    fgets(buf, 1023, stdin);
}

struct ConnectInfo {
    int sock_meta;
    int sock_feature;
    int sock_video;
};

static int listen_connection(int port) {
    sockaddr_in my_addr;
    int sockoptval = 1;

    int svc;
    if ((svc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to open listen socket");
        exit(EXIT_FAILURE);
    }

    // set port resuable
    setsockopt(svc, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));

    memset((void*) &my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(svc, (struct sockaddr*) &my_addr, sizeof(my_addr)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    if (listen(svc, MAX_N_SENSOR) < 0) {
        perror("Failed to listen");
        exit(EXIT_FAILURE);
    }
    return svc;
}


static void wait_connection(int svc, vector<ConnectInfo>& client) {
    int rqst;

    fd_set readfds;
    while(true) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(svc, &readfds);
        if (select(svc + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("Failed to select");
            exit(EXIT_FAILURE);
        }

        sockaddr_in client_addr;
        socklen_t alen = sizeof(client_addr);

        if (FD_ISSET(svc, &readfds)) {
            if ((rqst = accept(svc, (struct sockaddr*) &client_addr, &alen)) < 0) {
                if (rqst == EWOULDBLOCK) continue;
                perror("Failed to accept");
                exit(EXIT_FAILURE);
            }

            printf("Accept connection from: %s:%d, fd = %d\n",
                    inet_ntoa(client_addr.sin_addr), (int) ntohs(client_addr.sin_port), rqst);
            ConnectInfo c;
            c.sock_meta = rqst;
            c.sock_feature = connect_to(client_addr.sin_addr.s_addr, PORT_FEATURE);
            printf(">>> Connect to feature channel, fd = %d\n", c.sock_feature);
            c.sock_video = connect_to(client_addr.sin_addr.s_addr, PORT_VIDEO);
            printf(">>> Connect to video channel, fd = %d\n", c.sock_video);
            client.push_back(c);
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            wait_enter();
            break;
        }
    }
}

typedef std::pair<int, char*> Feature;

class FeatureSender {
public:
    FeatureSender(std::vector<ConnectInfo>& info)
            :_queue(new ConcurrentQueue<Feature>()), _info(&info) {}

    void start() {
        _thread = new std::thread(run, _queue, _info);
    }

    void stop() {
        Feature f;
        f.first = 0;
        f.second = 0;
        _queue->push(f);
        _thread->join();
    }

    void broadcastFeature(Feature & f) {
        _queue->push(f);
    }
private:
    ConcurrentQueue<Feature>* _queue;
    std::vector<ConnectInfo>* _info;
    std::thread* _thread;

    static void run(ConcurrentQueue<Feature>* queue, std::vector<ConnectInfo>* info) {
        Feature f;
        while (true) {
            f = queue->pop();
            if (!f.second) break;
            int vid = f.first;
            for (int i=0, n=info->size(); i<n; ++i) {
                if (i == vid) continue;
                if (sendall(info->operator[](i).sock_feature, f.second, INTER_FEATURE_SIZE) != INTER_FEATURE_SIZE) {
                    perror("Failed to send feature");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
};

class Sensor {
public:
    Sensor(ConnectInfo& info, int vid, FeatureSender& sender)
            :_info(info), _vid(vid), _sender(&sender) {}

    void start() {
        char buf[128];
        FILE* infofile;

        sprintf(buf, "info-%d.txt", _vid);
        infofile = fopen(buf, "w");
        if (!infofile) {
            perror("Failed to open info file");
            exit(EXIT_FAILURE);
        }
        _msg_thread = new std::thread(run_msg, _info.sock_meta, infofile, _vid);

        sprintf(buf, "video-%d.264", _vid);
        infofile = fopen(buf, "wb");
        if (!infofile) {
            perror("Failed to open info file");
            exit(EXIT_FAILURE);
        }
        _video_thread = new std::thread(run_video, _info.sock_video, infofile);

        _feature_thread = new std::thread(run_feature, _info.sock_feature, _vid, _sender);

        char msg = MSG_START;
        if (sendall(_info.sock_meta, &msg, 1) < 0) {
            perror("Failed to send msg");
            exit(EXIT_FAILURE);
        }
    }

    void stop() {
        char msg = MSG_STOP;
        if (sendall(_info.sock_meta, &msg, 1) < 0) {
            perror("Failed to send msg");
            exit(EXIT_FAILURE);
        }
        _msg_thread->join();
        _video_thread->join();
        _feature_thread->join();
    }
private:
    ConnectInfo _info;
    int _vid;
    std::thread* _video_thread;
    std::thread* _feature_thread;
    std::thread* _msg_thread;
    FeatureSender* _sender;

    static void run_msg(int socket, FILE* ofile, int vid) {
        uint32_t buf[3];
        int pkg_size = sizeof(uint32_t) * 3;
        int n;
        while ((n = recvall(socket, (char*) &buf, pkg_size)) == pkg_size) {
            printf("%u %u %u\n", buf[0], buf[1], buf[2]);
            fprintf(ofile, "%u %u %u\n", buf[0], buf[1], buf[2]);
            fflush(ofile);
        }
        if (n < 0) {
            perror("Failed to receive msg");
        } else {
            printf("Sensor %d: msg socket closed by sensor(%d)\n", vid, n);
        }

        fclose(ofile);
    }

    static void run_video(int socket, FILE* ofile) {
        uint8_t buf[STREAM_PACKET_SIZE];
        int n;
        while ((n = recv(socket, buf, STREAM_PACKET_SIZE, 0)) > 0) {
            fwrite(buf, n, 1, ofile);
            fflush(ofile);
        }
        if (n < 0) {
            perror("Failed to receive video");
        }
        fclose(ofile);
    }

    static void run_feature(int socket, int vid, FeatureSender* sender) {
        static char data[INTER_FEATURE_SIZE];
        int n;
        while ((n = recvall(socket, data, INTER_FEATURE_SIZE)) == INTER_FEATURE_SIZE) {
            printf("Feature Received, vid = %d\n", vid);
            char* buf = new char[INTER_FEATURE_SIZE];
            memcpy(buf, data, INTER_FEATURE_SIZE);
            Feature f;
            f.first = vid;
            f.second = buf;
            sender->broadcastFeature(f);
        }
        if (n < 0) {
            perror("Failed to receive feature");
        }
    }
};

int main(int argc, char *argv[]) {
    printf("Listen to port: %d\n", PORT);
    int sock_fd = listen_connection(PORT);

    vector<ConnectInfo> client_fds;
    printf("Type 'start' to begin the system.\n");
    printf("Wait for connection...\n");
    wait_connection(sock_fd, client_fds);

    int n_sensor = client_fds.size();

    FeatureSender featureSender(client_fds);
    featureSender.start();

    vector<Sensor*> sensors;
    for (int i=0; i<n_sensor; ++i) {
        sensors.push_back(new Sensor(client_fds[i], i, featureSender));
    }

    printf("Sending start signal...\n");
    for (int i=0; i<n_sensor; ++i) {
        sensors[i]->start();
    }

    wait_enter();

    featureSender.stop();

    printf("Sending stop signal...\n");
    for (int i=0; i<n_sensor; ++i) {
        sensors[i]->stop();
    }
}
