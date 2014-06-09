#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include "gmm/Sensor.h"
// #include "gmm/Sender.h"
//
// #include "rpi/rpi_opencv/RpiVideoCapture.h"
// #include "rpi/rpi_opencv/RpiVideoWriter.h"
//
// #include <fcntl.h>
// #include <stdio.h>
// #include <sys/time.h>
//
// #include <opencv2/opencv.hpp>
// using namespace cv;
//
// class RpiSender: public Sender {
// public:
//     RpiSender() {}
//
//     virtual void sendFrame(InputArray frame, size_t time, int idx) {}
//
//     virtual void sendFeature(InputArray iFeature, double score, size_t time, int idx) {}
// private:
// };
//
// static size_t gettime() {
//     struct timeval tv;
//     gettimeofday(&tv, NULL);
//     return tv.tv_sec * 1000 + tv.tv_usec / 1000;
// }



int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: %s <server ip> <server port>\n", argv[0]);
        return -1;
    }

    int port = atoi(argv[2]);

    int sockfd;
    int sockoptval = 1;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to open socket");
        exit(EXIT_FAILURE);
    }

    sockaddr_in serveraddr;
    memset((void*) &serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    if (inet_aton(argv[1], &(serveraddr.sin_addr)) < 0) {
        printf("Incorrect IP address: %s\n", argv[1]);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) < 0) {
        perror("Failed to connect");
        exit(EXIT_FAILURE);
    }


//     const int width = 320;
//     const int height = 240;
//
//     if (argc != 2) {
//         printf("usage: %s <output_file>\n", argv[0]);
//         return -1;
//     }
//
//     RpiVideoCapture& cap = RpiVideoCapture::getInstance();
//     cap.init(width, height);
//
//     int o_fd = open(argv[1], O_WRONLY | O_CREAT,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
//     if (o_fd < 0) {
//         perror("Failed to open output file");
//         exit(EXIT_FAILURE);
//     }
//     RpiVideoWriter& writer = RpiVideoWriter::getInstance();
//     writer.init(width, height, 30, o_fd);
//
//     RpiSender sender;
//     Sensor sensor(&sender, false);
//
//     list<ReceivedFeature> features;
//     Mat frame;
//     int idx = 0;
//     while (cap.read(frame)) {
//         sensor.next(idx, frame, gettime(), features);
//         idx++;
//     }
//     sensor.finish();
}
