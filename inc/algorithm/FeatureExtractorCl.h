#ifndef FEATURE_EXTRACTOR_CL
#define FEATURE_EXTRACTOR_CL

#include "FeatureExtractor.h"

/**
 * Color Layout Descriptor
 */

class FeatureExtractorCl : public FeatureExtractor {
public:
    FeatureExtractorCl(int n_block = 8):_block_count(n_block) {}
    void extract(InputArray frame, OutputArray feature);
private:
    int _block_count;
};
#endif
