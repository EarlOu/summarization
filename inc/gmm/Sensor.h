#ifndef SENSOR_H
#define SENSOR_H

#include "gmm/OnlineClusterMog.h"
#include "algorithm/FeatureExtractorCl.h"

#include <opencv2/opencv.hpp>
#include <list>
using std::list;

#define N_BLOCK 8

struct ReceivedFeature {
    cv::Mat feature;
    float score;
    size_t time;
    ReceivedFeature(cv::Mat _feature, float _s, size_t _t): feature(_feature), score(_s), time(_t) {}
};

struct BufferedFeature : public ReceivedFeature {
    cv::Mat frame;
    int idx;
    BufferedFeature(cv::Mat _feature, float _s, size_t _t, cv::Mat _frm, int _idx)
        :ReceivedFeature(_feature, _s, _t), frame(_frm), idx(_idx) {}
};

class Sensor {
public:
    Sensor(bool intra_only): INTRA_ONLY(intra_only), _featureExtractor(N_BLOCK), _idx(0) {}
    void next(cv::InputArray iFrame, size_t time, list<ReceivedFeature>& features);
    void finish();
protected:
    virtual void obtainReceivedFeature(list<ReceivedFeature>& features) = 0;
    virtual void sendFrame(cv::InputArray frame, int idx) = 0;
private:
    bool INTRA_ONLY;
    FeatureExtractorCl _featureExtractor;
    OnlineClusterMog _onlineCluster;
    int _idx;
    list<BufferedFeature> _buf;
};

#endif
