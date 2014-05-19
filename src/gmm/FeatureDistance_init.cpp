#include "FeatureDistance.h"
#include "FeatureDistanceNorm.h"
#include "FeatureDistanceConcatNorm.h"
#include "util.h"

CV_INIT_ALGORITHM(FeatureDistanceNorm, "FeatureDistance.NORM",
    obj.info()->addParam(obj, "normType", obj._normType));
CV_INIT_ALGORITHM(FeatureDistanceConcatNorm, "FeatureDistance.CONCAT_NORM",
    obj.info()->addParam(obj, "normType", obj._normType));

bool initModule_FeatureDistance()
{
    Ptr<Algorithm> p1 = createFeatureDistanceNorm();
    Ptr<Algorithm> p2 = createFeatureDistanceConcatNorm();
    return p1->info() != NULL && p2->info() != NULL;
}