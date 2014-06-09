#include "rpi/common/Common.h"
#include "rpi/common/NetUtil.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

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
    if (argc != 2) {
        printf("usage: %s <server ip>\n", argv[0]);
        return -1;
    }

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
    printf("Successfully create feature channel, fd=%d\n", fd_video);

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
