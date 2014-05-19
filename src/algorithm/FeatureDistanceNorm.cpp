#include "algorithm/FeatureDistanceNorm.h"

double FeatureDistanceNorm::diff(InputArray feature1, InputArray feature2)
{
    return norm(feature1, feature2, _normType);
}