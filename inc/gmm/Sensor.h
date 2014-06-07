#ifndef SENSOR_H
#define SENSOR_H

#include "Sender.h"
#include "OnlineClusterMog.h"
#include "algorithm/FeatureExtractorCl.h"

#include <opencv2/opencv.hpp>
#include <list>
using std::list;

#define N_BLOCK 8

struct ReceivedFeature {
    cv::Mat feature;
    double score;
    size_t time;
    ReceivedFeature(cv::Mat _feature, double _s, size_t _t): feature(_feature), score(_s), time(_t) {}
};

struct BufferedFeature : public ReceivedFeature {
    cv::Mat frame;
    int idx;
    BufferedFeature(cv::Mat _feature, double _s, size_t _t, cv::Mat _frm, int _idx)
        :ReceivedFeature(_feature, _s, _t), frame(_frm), idx(_idx) {}
};

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
};

#endif
