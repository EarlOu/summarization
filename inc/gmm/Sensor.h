#ifndef SENSOR_H
#define SENSOR_H

#include <opencv2/opencv.hpp>
#include <list>
using std::list;

class Sensor {
public:
    Sensor(bool inter_stage): _inter_stage(inter_stage) {}
    void run();

    struct ReceivedFeature {
        cv::Mat feature;
        float score;
        size_t time;
    };

protected:
    virtual bool read(cv::Mat & frame, size_t & time) = 0;
    virtual void obtainReceivedFeature(cv::vector<ReceivedFeature>& features) = 0;
    virtual void sendFrame(cv::InputArray frame, int idx) = 0;
private:
    bool _inter_stage;
    struct BufferedFrame {
        cv::Mat frame;
        cv::Mat feature;
        float score;
        size_t time;
        int idx;
        BufferedFrame(cv::Mat _frm, cv::Mat _feature, float _s, int _idx, size_t _t)
                : frame(_frm), feature(_feature), score(_s), idx(_idx), time(_t) {}
    };

    list<BufferedFrame> _buf;
};

#endif
