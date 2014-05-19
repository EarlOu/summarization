#ifndef FEATURE_DISTANCE
#define FEATURE_DISTANCE

#include <opencv2/opencv.hpp>

using namespace cv;

class FeatureDistance : public Algorithm
{
public:
    virtual double diff(InputArray feature1, InputArray feature2) = 0;
};
bool initModule_FeatureDistance();
#endif