#include "sensor/summarization.h"

#include <opencv2/opencv.hpp>
using namespace cv;

#define N_BLOCK_COLOR_LAYOUT 8

void extractColorLayout(InputArray iFrame, OutputArray oFeature) {
    static const int feature_dim = N_BLOCK_COLOR_LAYOUT * N_BLOCK_COLOR_LAYOUT * 3;
    Mat frame = iFrame.getMat();
    oFeature.create(feature_dim, 1, CV_32FC1);
    Mat feature = oFeature.getMat();

    Mat f = frame;

    int w = f.size().width;
    int h = f.size().height;

    int block_width = w / N_BLOCK_COLOR_LAYOUT;
    int block_height = h / N_BLOCK_COLOR_LAYOUT;
    int crop_w = block_width * N_BLOCK_COLOR_LAYOUT;
    int crop_h = block_height * N_BLOCK_COLOR_LAYOUT;

    int index = 0;
    for (int y=0; y<crop_h; y+=block_height) {
        for (int x=0; x<crop_w; x+=block_width) {
            Scalar s = mean(f.colRange(x, x + block_width).
                    rowRange(y, y + block_height));
            feature.at<float>(index++) = float((int) round(s[0])) / 255;
            feature.at<float>(index++) = float((int) round(s[1])) / 255;
            feature.at<float>(index++) = float((int) round(s[2])) / 255;
        }
    }
}
