#ifndef ONLINE_CLUSTER_MOG
#define ONLINE_CLUSTER_MOG

#include "OnlineCluster.h"

class OnlineClusterMog : public OnlineCluster {
public:
    // BL2
    OnlineClusterMog():_K(9), _alpha(0.004), _T(0.5), _INIT_SIGMA(0.2) {};
    AlgorithmInfo* info() const;
    int cluster(InputArray feature, OutputArray residue);

    bool isBackground(int cluster);
    void getCenter(int cluster, OutputArray center);
    int getTrueBackground(InputArray iFeature);
    void getBackground(OutputArray back);
private:
    class Cluster {
    public:
        Mat mean;
        double sigma;
        double weight;
        int index;
    };

    int _K;
    double _alpha;
    vector<Cluster> _cluster;
    double _T;
    double _INIT_SIGMA;

    class ClusterComparator {
    public:
        bool operator() (const Cluster& i, const Cluster& j) {
            double l1 = i.weight / i.sigma;
            double l2 = j.weight / j.sigma;
            return l1 > l2;
        }
    };
};

#endif
