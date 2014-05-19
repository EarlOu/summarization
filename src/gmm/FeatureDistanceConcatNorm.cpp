#include "FeatureDistanceConcatNorm.h"

double FeatureDistanceConcatNorm::diff(InputArray iFeature1, InputArray iFeature2)
{
    Mat feature1 = iFeature1.getMat();
    Mat feature2 = iFeature2.getMat();
    double diff = 0.0;
    int index = 0;
    for (int i=0; i<_numFeature; ++i)
    {
        int l = _featureLength[i];
        double w = _featureWeight[i];

        Mat f1 = feature1.rowRange(index, index+l);
        Mat f2 = feature2.rowRange(index, index+l);
        diff += norm(f1, f2, _normType) * w;
        index += l;
    }
    return diff;
}