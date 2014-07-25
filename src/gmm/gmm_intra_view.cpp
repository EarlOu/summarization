#include "gmm/Sensor.h"
#include "gmm/Sender.h"

#include <stdio.h>
#include <assert.h>
#include <opencv2/opencv.hpp>
using namespace cv;

#define FPS 30
#define MINI_SEC_PER_FRAME 33

class IntraSimulateSender: public Sender {
public:
    IntraSimulateSender(): _skim_start(false), _skim_start_idx(0), _last_received_idx(0) {}

    virtual void sendFrame(InputArray frame, uint32_t time, int idx) {
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

    virtual void sendFeature(InputArray iFeature, float score, uint32_t time, int idx) {}

    void finish() {
        if (_skim_start) {
            printf("%d %d\n", _skim_start_idx, _last_received_idx + 1);
        }
    }
private:
    bool _skim_start;
    int _skim_start_idx;
    int _last_received_idx;
};


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s <input_video>\n", argv[0]);
        return -1;
    }

    VideoCapture cap(argv[1]);
    IntraSimulateSender sender;
    Sensor sensor(&sender, FPS, 9, 0.004, 0.5, 0.005, true);

    list<FeaturePacket> features;
    Mat frame;
    int idx = 0;
    uint32_t time = 0;
    while (cap.read(frame)) {
        sensor.next(idx, frame, time, features);
        idx++;
        time += FPS;
    }
    sensor.finish();
    sender.finish();
}
