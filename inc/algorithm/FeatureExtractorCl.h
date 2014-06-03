#ifndef FEATURE_EXTRACTOR_CL
#define FEATURE_EXTRACTOR_CL

#include "FeatureExtractor.h"

/**
 * Color Layout Descriptor
 */

class FeatureExtractorCl : public FeatureExtractor {
public:
    FeatureExtractorCl():_block_count(8) {}

    void extract(InputArray frame, OutputArray feature);
    AlgorithmInfo* info() const;

private:
    int _block_count;
};
#endif
