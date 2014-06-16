#ifndef SENDER_H
#define SENDER_H

#include <inttypes.h>
#include <opencv2/opencv.hpp>

class Sender {
public:
    virtual void sendFrame(cv::InputArray frame, uint32_t time, int idx) = 0;
    virtual void sendFeature(cv::InputArray iFeature, float score, uint32_t time, int idx) = 0;
};
#endif
