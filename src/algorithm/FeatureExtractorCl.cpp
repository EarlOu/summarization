#include "algorithm/FeatureExtractorCl.h"
#include <math.h>

void FeatureExtractorCl::extract(InputArray iFrame, OutputArray oFeature) {
    Mat frame = iFrame.getMat();
    oFeature.create(_block_count * _block_count * 3, 1, CV_32FC1);
    Mat feature = oFeature.getMat();

    Mat f;
    // cvtColor(frame, f, CV_BGR2HSV);
    f = frame.clone();

    int w = f.size().width;
    int h = f.size().height;

    int w_step = w / _block_count;
    int h_step = h / _block_count;
    int crop_w = w_step * _block_count;
    int crop_h = h_step * _block_count;

    int index = 0;
    for (int y=0; y<crop_h; y+=h_step) {
        for (int x=0; x<crop_w; x+=w_step) {
            Scalar s = mean(f.colRange(x, x + w_step).
                    rowRange(y, y + h_step));
            feature.at<float>(index + 0) = float((int) round(s[0])) / 255;
            feature.at<float>(index + 1) = float((int) round(s[1])) / 255;
            feature.at<float>(index + 2) = float((int) round(s[2])) / 255;
            index += 3;
        }
    }
}
