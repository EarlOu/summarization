#ifndef FEATURE_EXTRACTOR_EDGE
#define FEATURE_EXTRACTOR_EDGE

#include "FeatureExtractor.h"

/**
 * Local Color Histogram (LCH) extractor
 */

class FeatureExtractorEdge : public FeatureExtractor
{
public:
    FeatureExtractorEdge();

    void extract(InputArray frame, OutputArray feature);

    AlgorithmInfo* info() const;
private:
    // parameters
    double _edge_th;

    vector<Mat> _kernel;
};
#endif