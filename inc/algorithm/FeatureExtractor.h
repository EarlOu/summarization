#ifndef FEATURE_EXTRACTOR
#define FEATURE_EXTRACTOR

#include <opencv2/opencv.hpp>
using namespace cv;

class FeatureExtractor {
public:
    virtual void extract(InputArray frame, OutputArray feature) = 0;
};
#endif
