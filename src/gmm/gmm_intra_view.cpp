#include "gmm/Sensor.h"

#include <assert.h>
#include <opencv2/opencv.hpp>
using namespace cv;

#define FPS 30

class IntraStageSimulateSensor:public Sensor {
public:
    IntraStageSimulateSensor()
            :Sensor(true), _skim_start(false), _skim_start_idx(0), _last_received_idx(0) {}

    void finish() {
        Sensor::finish();
        if (_skim_start) {
            printf("%d %d\n", _skim_start_idx, _last_received_idx + 1);
        }
    }

private:
    bool _skim_start;
    int _skim_start_idx;
    int _last_received_idx;

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

    virtual void obtainReceivedFeature(list<ReceivedFeature>& features) {}
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s <input_video>\n", argv[0]);
        return -1;
    }

    VideoCapture cap(argv[1]);
    IntraStageSimulateSensor sensor;
    list<ReceivedFeature> features;

    Mat frame;
    size_t time = 0;
    while (cap.read(frame)) {
        sensor.next(frame, time, features);
        time += FPS;
    }
    sensor.finish();
}
