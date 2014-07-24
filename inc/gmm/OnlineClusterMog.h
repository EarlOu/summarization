#ifndef ONLINE_CLUSTER_MOG
#define ONLINE_CLUSTER_MOG

#include "OnlineCluster.h"

class OnlineClusterMog : public OnlineCluster {
public:
    OnlineClusterMog(int K, double alpha, double T, double init_sigma)
            :_K(K), _alpha(alpha), _T(T), _INIT_SIGMA(init_sigma) {};
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

    const int _K;
    const double _alpha;
    const double _T;
    const double _INIT_SIGMA;
    vector<Cluster> _cluster;

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
