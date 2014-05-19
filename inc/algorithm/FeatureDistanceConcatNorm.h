#ifndef FEATURE_DISTANCE_CONCAT_NORM
#define FEATURE_DISTANCE_CONCAT_NORM

#include "FeatureDistance.h"

class FeatureDistanceConcatNorm : public FeatureDistance
{
public:
    FeatureDistanceConcatNorm():_normType(NORM_L1), _numFeature(0) {}
    AlgorithmInfo* info() const;

    double diff(InputArray feature1, InputArray feature2);

    void setFeatureNum(int numFeature, vector<int>& featureLength, vector<double>& featureWeight) {
        _numFeature = numFeature;
        _featureLength = featureLength;
        _featureWeight = featureWeight;
    }
private:
    int _normType;
    int _numFeature;
    vector<int> _featureLength;
    vector<double> _featureWeight;
};
#endif