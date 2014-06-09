#include <opencv2/opencv.hpp>
using namespace cv;

#include "sensor/summarization.h"
#include "sensor/OnlineClusterMog.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s <input_video>\n", argv[0]);
        return -1;
    }
    VideoCapture cap(argv[1]);
    OnlineClusterMog mog;

    Mat frame;
    Mat feature;
    Mat residue;
    while (cap.read(frame)) {
        extractColorLayout(frame, feature);
        int cluster = mog.cluster(feature, residue);
        bool choose = !mog.isBackground(cluster);
        printf("%d\n", choose ? 1 : 0);
    }
}
