#include "gmm/util.h"

#include <stdio.h>
#include <math.h>
#include <opencv2/opencv.hpp>
using namespace cv;

void parseInput(const char* path, vector<VideoSensor>& sensors)
{
    string dirname = getDirname(path);
    FILE* ifile = fopen(path, "r");
    int numOfVideo;
    fscanf(ifile, "%d", &numOfVideo);
    for (int i=0; i<numOfVideo; ++i)
    {
        char buf[128];
        int offset, length;
        fscanf(ifile, "%s %d %d", buf, &offset, &length);

        string video = dirname + string("/") + string(buf);
        VideoCapture cap(video);

        if (!cap.isOpened())
        {
            perror("Failed to open video file.");
            exit(-1);
        }

        sensors.push_back(VideoSensor(cap, offset));
    }

    fclose(ifile);
}

string getDirname(const string& str)
{
    size_t found;
    found = str.find_last_of("/\\");
    return str.substr(0, found);
}
