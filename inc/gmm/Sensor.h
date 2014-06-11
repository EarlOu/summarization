#ifndef SENSOR_H
#define SENSOR_H

#include "Sender.h"
#include "Common.h"
#include "OnlineClusterMog.h"
#include "algorithm/FeatureExtractorCl.h"

#include <opencv2/opencv.hpp>
#include <list>
using std::list;

#define N_BLOCK 8

class Sensor {
public:
    Sensor(Sender* sender, bool intra_only = false)
            : _sender(sender), INTRA_ONLY(intra_only), _featureExtractor(N_BLOCK) {}
    void next(int idx, cv::InputArray iFrame, size_t time, list<ReceivedFeature>& features);
    void finish();
private:
    Sender* _sender;
    bool INTRA_ONLY;
    FeatureExtractorCl _featureExtractor;
    OnlineClusterMog _onlineCluster;
    list<BufferedFeature> _buf;

    // disable default copy constructor and assign operator
    Sensor(const Sensor&) {}
    void operator=(const Sensor&) {}
};

#endif
