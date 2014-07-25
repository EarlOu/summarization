#ifndef SENSOR_H
#define SENSOR_H

#include "Sender.h"
#include "Common.h"
#include "OnlineClusterMog.h"
#include "algorithm/FeatureExtractorCl.h"

#include <inttypes.h>
#include <opencv2/opencv.hpp>
#include <list>
using std::list;

#define N_BLOCK 8

class Sensor {
public:
    Sensor(Sender* sender, int fps,
            int K, double alpha, double T, double init_sigma,
            bool intra_only = false, int time_to_ignore = 0)
            : _sender(sender), FPS(fps), _onlineCluster(K, alpha, T, init_sigma), _featureExtractor(N_BLOCK),
              INTRA_ONLY(intra_only), TIME_TO_IGNORE(time_to_ignore) {}

    void next(int idx, cv::InputArray iFrame, uint32_t time, list<FeaturePacket>& features);
    void finish();
private:
    Sender* _sender;
    FeatureExtractorCl _featureExtractor;
    OnlineClusterMog _onlineCluster;
    list<BufferedFeature> _buf;
    const bool INTRA_ONLY;
    const int TIME_TO_IGNORE;
    const int FPS;

    // disable default copy constructor and assign operator
    Sensor(const Sensor&): FPS(30), _onlineCluster(0, 0, 0, 0), INTRA_ONLY(false), TIME_TO_IGNORE(0) {}
    void operator=(const Sensor&) {}
};

#endif
