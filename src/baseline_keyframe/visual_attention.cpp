#include "database/Dataset.h"

#include <opencv2/opencv.hpp>
using namespace cv;

#include <stdio.h>
#include <limits.h>
#include <deque>
using namespace std;

int main(int argc, char *argv[]) {
    printf("Compute visual attention score for each frame in the dataset\n");
    if (argc != 3 && argc != 4) {
        printf("usage: %s <dataset_dir> <score_dir> [scale]\n", argv[0]);
        return -1;
    }
    int scale = 1;
    if (argc == 4) {
        scale = atoi(argv[3]);
    }

    Dataset set(argv[1]);
    vector<VideoInfo>& info = set.getVideoInfo();

    for (size_t vid=0; vid<info.size(); ++vid) {
        VideoCapture cap(info[vid].path);

        Mat prev_frame, frame;
        double score_d1 = 0, score_d2 = INT_MAX;
        int idx = 0;
        char buf[128];
        sprintf(buf, "%s/%d.txt", argv[2], (int) vid);
        FILE* file = fopen(buf, "w");
        while (cap.read(frame)) {
            printf("%d: %d\n", (int) vid, idx);
            cvtColor(frame, frame, CV_BGR2GRAY);
            if (scale!=1) {
                resize(frame, frame, Size(), 1.0/scale, 1.0/scale);
            }
            if (prev_frame.empty()) {
                prev_frame = frame.clone();
                idx++;
                continue;
            }

            Mat flow;
            calcOpticalFlowFarneback(prev_frame, frame, flow, 0.5, 1, 8, 3, 5, 1.2, 0);

            vector<Mat> flows;
            split(flow, flows);
            Mat flow_x = flows[0];
            Mat flow_y = flows[1];
            Mat flow_mag, flow_ang;
            cartToPolar(flow_x, flow_y, flow_mag, flow_ang);
            double score = mean(flow_mag)[0];
            fprintf(file, "%lf\n", score);
            prev_frame = frame.clone();
            idx++;
        }
        fclose(file);
    }
}
