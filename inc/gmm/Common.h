#ifndef GMM_COMMON_H
#define GMM_COMMON_H

#include <opencv2/opencv.hpp>

#define INTER_FEATURE_DIM 16

const int INTER_FEATURE_SIZE =
        INTER_FEATURE_DIM * sizeof(float) + // feature
        sizeof(float) + // score
        sizeof(int32_t); // time

struct FeaturePacket {
    cv::Mat feature;
    double score;
    size_t time;

    FeaturePacket(cv::Mat _feature, double _s, size_t _t): feature(_feature), score(_s), time(_t) {}
};

struct BufferedFeature : public FeaturePacket {
    cv::Mat frame;
    int idx;
    BufferedFeature(cv::Mat _feature, double _s, size_t _t, cv::Mat _frm, int _idx)
        :FeaturePacket(_feature, _s, _t), frame(_frm), idx(_idx) {}
};

#endif
