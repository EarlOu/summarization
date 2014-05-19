#ifndef SERVER_H
#define SERVER_H

#include "VideoSensor.h"
#include "Shot.h"
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using std::vector;

class Server
{

public:
    Server(vector<VideoSensor>& sensors, VideoWriter& writer):
            _sensors(sensors),
            _writer(writer) {};

    void run();
private:
    vector<VideoSensor> _sensors;
    vector<vector<Shot> > _shots;
    VideoWriter& _writer;

    void nextSelection(vector<bool>& selects);
    void transmitFrame(const vector<bool>& selects, int index);
    int computeTotalLength();
    void createShotArray();
    void closeShot(int index);
    void postprocess(vector<vector<vector<Shot> > >& shots);
    void write(vector<vector<vector<Shot> > >& shots);
};

#endif