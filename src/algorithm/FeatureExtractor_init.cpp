#include "algorithm/FeatureExtractor.h"
#include "algorithm/FeatureExtractorLch3D.h"
#include "algorithm/FeatureExtractorLch.h"
#include "algorithm/FeatureExtractorEdge.h"
#include "algorithm/FeatureExtractorCl.h"
#include "algorithm/util.h"

CV_INIT_ALGORITHM(FeatureExtractorLch3D, "FeatureDistance.LCH3D",
    obj.info()->addParam(obj, "h_bit", obj._h_bit);
    obj.info()->addParam(obj, "s_bit", obj._s_bit);
    obj.info()->addParam(obj, "v_bit", obj._v_bit);
    obj.info()->addParam(obj, "num_block_x", obj._num_block_x);
    obj.info()->addParam(obj, "num_block_y", obj._num_block_y));

CV_INIT_ALGORITHM(FeatureExtractorLch, "FeatureDistance.LCH",
    obj.info()->addParam(obj, "h_bit", obj._h_bit);
    obj.info()->addParam(obj, "s_bit", obj._s_bit);
    obj.info()->addParam(obj, "v_bit", obj._v_bit);
    obj.info()->addParam(obj, "num_block_x", obj._num_block_x);
    obj.info()->addParam(obj, "num_block_y", obj._num_block_y));

CV_INIT_ALGORITHM(FeatureExtractorEdge, "FeatureDistance.Edge",
    obj.info()->addParam(obj, "edge_th", obj._edge_th););

CV_INIT_ALGORITHM(FeatureExtractorCl, "FeatureDistance.Cl",);

bool initModule_FeatureExtractor()
{
    Ptr<Algorithm> ptr = createFeatureExtractorLch3D();
    if (ptr->info() == NULL) return false;

    ptr = createFeatureExtractorEdge();
    if (ptr->info() == NULL) return false;

    ptr = createFeatureExtractorLch();
    if (ptr->info() == NULL) return false;

    ptr = createFeatureExtractorCl();
    if (ptr->info() == NULL) return false;

    return true;
}