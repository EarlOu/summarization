#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include "util.h"
#include "Server.h"

using namespace cv;
using std::vector;

int main(int argc, char** argv)
{
    srand(time(NULL));
    if (argc != 2)
    {
        printf("usage: %s <file_list.txt>\n", argv[0]);
        return -1;
    }

    vector<VideoSensor> sensors;
    parseInput(argv[1], sensors);

    if (sensors.size() == 0)
    {
        printf("There is no video files found.\n");
        return -1;
    }

    int w = sensors[0].getWidth();
    int h = sensors[0].getHeight();
    VideoWriter writer("output.avi", CV_FOURCC('M', 'P', 'E', 'G'), 30, Size(w, h));

    Server server(sensors, writer);
    server.run();

    return 0;
}

