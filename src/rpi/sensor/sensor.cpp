#include "sensor/OnlineClusterMog.h"
#include "sensor/summarization.h"
#include <stdio.h>
#include <sys/time.h>

#ifndef SIMULATE
// header for RPI hardware
#include "rpi_opencv/RpiVideoCapture.h"
#include "rpi_opencv/RpiVideoWriter.h"
#include <fcntl.h>
#endif

#include <opencv2/opencv.hpp>
using namespace cv;

int main(int argc, char *argv[]) {

#ifdef SIMULATE
    if (argc != 3) {
        printf("usage: %s <video_file> <output_file>\n", argv[0]);
        return -1;
    }
    VideoCapture cap(argv[1]);
    VideoWriter writer(argv[2],
            cap.get(CV_CAP_PROP_FOURCC),
            cap.get(CV_CAP_PROP_FPS),
            Size(cap.get(CV_CAP_PROP_FRAME_WIDTH), cap.get(CV_CAP_PROP_FRAME_HEIGHT)));

#else
    const int width = 320;
    const int height = 240;
    if (argc != 2) {
        printf("usage: %s <output_file>\n", argv[0]);
        return -1;
    }

    RpiVideoCapture& cap = RpiVideoCapture::getInstance();
    cap.init(width, height);

    int o_fd = open(argv[1], O_WRONLY | O_CREAT,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (o_fd < 0) {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }
    RpiVideoWriter& writer = RpiVideoWriter::getInstance();
    writer.init(width, height, 30, o_fd);
#endif

    OnlineClusterMog onlineCluster;

    int idx = 0;
    Mat frame;
    Mat feature;
    Mat residue;
    struct timeval tv1, tv2;
    while (cap.read(frame)) {
        printf("Frame: %d\n", idx);

        // Intra-view stage
        extractColorLayout(frame, feature);
        int cluster = onlineCluster.cluster(feature, residue);
        bool choose = !onlineCluster.isBackground(cluster);

        if (choose) {
            writer.write(frame);
        }

        // calculate FPS
        if (idx != 0) {
            gettimeofday(&tv2, NULL);
            float sec = tv2.tv_sec - tv1.tv_sec + (tv2.tv_usec - tv1.tv_usec) / 1000000.0;
            float fps = 1.0f / sec;
            printf("FPS: %.2f\n", fps);
        }
        idx++;
        gettimeofday(&tv1, NULL);
    }
}
