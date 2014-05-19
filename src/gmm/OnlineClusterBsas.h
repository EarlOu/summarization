#ifndef ONLINE_CLUSTER_BSAS
#define ONLINE_CLUSTER_BSAS

#include "OnlineCluster.h"
#include "FeatureDistance.h"

/**
 * Basic Sequential Algorithm Scheme
 */
class OnlineClusterBsas : public OnlineCluster
{
public:
    OnlineClusterBsas():
            _featureDiff(Algorithm::create<FeatureDistance>("FeatureDistance.NORM")),
            _diffTh(0.5),
            _learningRate(0.08),
            _maxNumCluster(10),
            _lastSeenTh(900),
            _histReplaceTh(30),
            _backgroundHistTh(60),
            _mergeTh(0.3)
            {};
    int cluster(InputArray feature, OutputArray redidue);
    bool isBackground(int cluster);
    void getCenter(int cluster, OutputArray oCenter);
    bool isMatched(InputArray feature, int cluster);
    int getHistCount(int cluster);
    double diff(InputArray feature, int cluster);
    AlgorithmInfo* info() const;
private:
    Ptr<FeatureDistance> _featureDiff;
    double _diffTh;
    double _learningRate;
    int _maxNumCluster;
    int _lastSeenTh;
    int _histReplaceTh;
    int _backgroundHistTh;
    double _mergeTh;

    vector<Mat> _centroid;
    vector<int> _histCount;
    vector<int> _lastCount;
};
#endif