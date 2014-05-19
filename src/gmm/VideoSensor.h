#ifndef VIDEO_SENSOR_H
#define VIDEO_SENSOR_H

#include <opencv2/opencv.hpp>
#include "OnlineClusterMog.h"
#include "FeatureExtractorCl.h"
#include "FeatureDistanceNorm.h"
#include "Shot.h"

using namespace cv;

class VideoSensor
{
public:
    VideoSensor(VideoCapture& cap, int offset):
            _index(0),
            _cap(cap),
            _offset(offset),
            _length(cap.get(CV_CAP_PROP_FRAME_COUNT)) {};

    int getEndIndex() {
        return _offset + _length;
    }

    bool next(OutputArray frame, OutputArray feature, double& score);
    bool receiveFeature(vector<Mat>& features, vector<double>& scores);
    int getWidth() {return _cap.get(CV_CAP_PROP_FRAME_WIDTH);}
    int getHeight() {return _cap.get(CV_CAP_PROP_FRAME_HEIGHT);}
    void write(VideoWriter& writer, Shot& s);

private:
    int _index;
    VideoCapture _cap;
    int _offset;
    int _length;
    Mat _feature;
    double _score;

    FeatureExtractorCl _extractor;
    OnlineClusterMog _onlineCluster;
    FeatureDistanceNorm _featureDistance;
};
#endif