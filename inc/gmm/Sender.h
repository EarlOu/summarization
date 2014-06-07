#ifndef SENDER_H
#define SENDER_H

#include <opencv2/opencv.hpp>

class Sender {
public:
    virtual void sendFrame(cv::InputArray frame, size_t time, int idx) = 0;
    virtual void sendFeature(cv::InputArray iFeature, double score, size_t time, int idx) = 0;
};
#endif
