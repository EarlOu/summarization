#ifndef FEATURE_EXTRACTOR_EDGE
#define FEATURE_EXTRACTOR_EDGE

#include "FeatureExtractor.h"

/**
 * Edge Histogram extractor
 */

class FeatureExtractorEdge : public FeatureExtractor {
public:
    FeatureExtractorEdge();
    void extract(InputArray frame, OutputArray feature);
private:
    // parameters
    double _edge_th;
    vector<Mat> _kernel;
};
#endif
