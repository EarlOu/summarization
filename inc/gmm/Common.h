#ifndef GMM_COMMON_H
#define GMM_COMMON_H

#include <opencv2/opencv.hpp>

#define INTER_FEATURE_DIM 16

struct ReceivedFeature {
    cv::Mat feature;
    double score;
    size_t time;
    ReceivedFeature(cv::Mat _feature, double _s, size_t _t): feature(_feature), score(_s), time(_t) {}
};

struct BufferedFeature : public ReceivedFeature {
    cv::Mat frame;
    int idx;
    BufferedFeature(cv::Mat _feature, double _s, size_t _t, cv::Mat _frm, int _idx)
        :ReceivedFeature(_feature, _s, _t), frame(_frm), idx(_idx) {}
};

#endif
