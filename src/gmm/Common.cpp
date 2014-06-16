#include "gmm/Common.h"
#include <inttypes.h>
#include <opencv2/opencv.hpp>
using namespace cv;

void FeaturePacket::pack(char* buf) {
    float* ptr = (float*) buf;
    for (int i=0; i<INTER_FEATURE_DIM; ++i) {
        ptr[i] = feature.at<float>(i);
    }
    ptr[INTER_FEATURE_DIM] = score;
    ((uint32_t*) buf) [INTER_FEATURE_DIM + 1] = time;
}

void FeaturePacket::unpack(char* buf) {
    feature = Mat(INTER_FEATURE_DIM, 1, CV_32F);
    float* ptr = (float*) buf;
    for (int i=0; i<INTER_FEATURE_DIM; ++i) {
        feature.at<float>(i) = ptr[i];
    }
    score = ptr[INTER_FEATURE_DIM];
    time = ((uint32_t*) buf) [INTER_FEATURE_DIM + 1];
}
