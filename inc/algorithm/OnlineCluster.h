#ifndef ONLINE_CLUSTER
#define ONLINE_CLUSTER

#include <opencv2/opencv.hpp>

using namespace cv;

class OnlineCluster:public Algorithm
{
public:
    virtual int cluster(InputArray feature, OutputArray residue) = 0;
    virtual bool isBackground(int cluster) = 0;
    virtual void getCenter(int cluster, OutputArray center) = 0;
};

bool initModule_OnlineCluster();
#endif