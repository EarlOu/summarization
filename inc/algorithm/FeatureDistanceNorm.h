#ifndef FEATURE_DISTANCE_NORM
#define FEATURE_DISTANCE_NORM

#include "FeatureDistance.h"

class FeatureDistanceNorm : public FeatureDistance
{
public:
    FeatureDistanceNorm():_normType(NORM_L2) {}
    FeatureDistanceNorm(int normType):_normType(normType) {};
    double diff(InputArray feature1, InputArray feature2);
    AlgorithmInfo* info() const;
private:
    int _normType;
};
#endif