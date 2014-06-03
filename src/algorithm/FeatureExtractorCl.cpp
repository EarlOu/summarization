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

    int block_width = w / _block_count;
    int block_height = h / _block_count;

    for (int y=0; y<_block_count; y++) {
        for (int x=0; x<_block_count; x++) {
            Scalar s = mean(f.colRange(x * block_width, x * block_width + block_width).
                    rowRange(y * block_height, y * block_height + block_height));
            int index = (x + y * _block_count) * 3;
            feature.at<float>(index + 0) = float((int) round(s[0])) / 255;
            feature.at<float>(index + 1) = float((int) round(s[1])) / 255;
            feature.at<float>(index + 2) = float((int) round(s[2])) / 255;
        }
    }
}
