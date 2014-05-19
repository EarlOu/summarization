#ifndef FEATURE_EXTRACTOR_LCH_3D
#define FEATURE_EXTRACTOR_LCH_3D

#include "FeatureExtractor.h"

/**
 * Local Color Histogram (LCH) extractor
 */

class FeatureExtractorLch3D : public FeatureExtractor
{
public:
    FeatureExtractorLch3D():
        _h_bit(4),
        _s_bit(2),
        _v_bit(2),
        _num_block_x(2),
        _num_block_y(2) {}

    void extract(InputArray frame, OutputArray feature);
    AlgorithmInfo* info() const;
private:
    void extractBlockHist(InputArray block, OutputArray feature);

    /* parameters */
    int _h_bit;
    int _s_bit;
    int _v_bit;
    int _num_block_x;
    int _num_block_y;

    /* temp information */
    int _h_bin;
    int _s_bin;
    int _v_bin;
    int _dim;
};
#endif