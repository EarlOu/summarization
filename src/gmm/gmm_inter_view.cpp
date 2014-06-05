
#include "gmm/Sensor.h"

#include <assert.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace cv;

#include <list>
using std::list;

#define FPS 30

class SimulateSensor:public Sensor {
    SimulateSensor(VideoCapture & cap, int offset): Sensor(true), _cap(cap), _idx(-offset) {}
private:
    VideoCapture & _cap;
    int _idx;
    list<ReceivedFeature> _features;

    virtual bool read(cv::Mat & frame, size_t & time) {
        if (_idx >= 0) {
            time = _idx * FPS;
            _idx++;
            return _cap.read(frame);
        }
        frame = Mat();
        _idx++;
        time = 0;
        return true;
    }
    virtual void obtainReceivedFeature(list<ReceivedFeature>& features) {
        while (!_features.empty()) {
            features.push_back(_features.front());
            _features.pop_front();
        }
    }
    virtual void sendFrame(cv::InputArray frame, int idx) {
    }
}

class SimulateSensor:public Sensor {
public:
    SimulateSensor(VideoCapture & cap, FILE* ofile)
            :Sensor(false), _cap(cap), _ofile(ofile),
            _skim_start(false), _skim_start_idx(0), _last_received_idx(0) {}

    void finish() {
        if (_skim_start) {
            printf("%d %d\n", _skim_start_idx, _last_received_idx + 1);
        }
    }

private:
    VideoCapture & _cap;
    FILE* _ofile;
    bool _skim_start;
    int _skim_start_idx;
    int _last_received_idx;

    virtual bool read(Mat& frame, size_t & time) {
        static size_t read_idx = 0;
        time = read_idx * FPS;
        read_idx++;
        return _cap.read(frame);
    }

    virtual void sendFrame(InputArray frame, int idx) {
        assert(idx > _last_received_idx);
        if (_skim_start) {
            if (idx != _last_received_idx + 1) { // the shot is break
                printf("%d %d\n", _skim_start_idx, _last_received_idx + 1);
                _skim_start_idx = idx;
            }
        } else {
            _skim_start = true;
            _skim_start_idx = idx;
        }
        _last_received_idx = idx;
    }

    virtual void obtainReceivedFeature(vector<ReceivedFeature>& features) {}
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s <>\n", argv[0]);
        return -1;
    }
    VideoCapture cap(argv[1]);
    IntraStageSimulateSensor sensor(cap);
    sensor.run();
    sensor.finish();
}
