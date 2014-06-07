#include "gmm/Sensor.h"
#include "gmm/Sender.h"
#include "database/Dataset.h"

#include <assert.h>
#include <opencv2/opencv.hpp>
using namespace cv;

#define FPS 30
#define MINI_SEC_PER_FRAME 33

class Server {
public:
    Server(int n_sensor): _n_sensor(n_sensor), _features(n_sensor) {}

    void broadcastFeature(InputArray iFeature, double score, size_t time, int vid) {
        for (int i=0; i<_n_sensor; ++i) {
            if (i == vid) continue;
            _features[i].push_back(ReceivedFeature(iFeature.getMat(), score, time));
        }
    }

    list<ReceivedFeature>& getFeatures(int vid) {
        return _features[vid];
    }

    void clearBuf(int vid) {
        _features[vid].clear();
    }
private:
    int _n_sensor;
    vector<list<ReceivedFeature> > _features;
};

class SimulateSender:public Sender {
public:
    SimulateSender(Server& server, int vid)
            :_server(server), _vid(vid), _skim_start(false), _skim_start_idx(0), _last_received_idx(0) {}

    virtual void sendFrame(InputArray iFrame, size_t time, int idx) {
        assert(idx > _last_received_idx);
        if (_skim_start) {
            if (idx != _last_received_idx + 1) { // the shot is break
                printf("%d %d %d\n", _vid, _skim_start_idx, _last_received_idx + 1);
                _skim_start_idx = idx;
            }
        } else {
            _skim_start = true;
            _skim_start_idx = idx;
        }
        _last_received_idx = idx;
    }

    virtual void sendFeature(InputArray iFeature, double score, size_t time, int idx) {
        printf("send frame: %d\n", _vid);
        _server.broadcastFeature(iFeature, score, time, _vid);
    }

    virtual void finish() {
        if (_skim_start) {
            printf("%d %d %d\n", _vid, _skim_start_idx, _last_received_idx + 1);
        }
    }
private:
    Server & _server;
    int _vid;
    bool _skim_start;
    int _skim_start_idx;
    int _last_received_idx;
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s <dataset_dir>\n", argv[0]);
        return -1;
    }
    Dataset set(argv[1]);
    int n_sensor = set.getVideoInfo().size();

    Server server(n_sensor);
    vector<VideoCapture> caps;
    vector<int> offsets;
    vector<SimulateSender*> senders;
    vector<Sensor> sensors;

    for (int i=0; i<n_sensor; ++i) {
        VideoInfo info = set.getVideoInfo()[i];
        caps.push_back(VideoCapture(info.path));
        offsets.push_back(info.offset);
        senders.push_back(new SimulateSender(server, i));
        sensors.push_back(Sensor(senders[i]));
    }

    int idx = 0;
    size_t time = 0;
    while (true) {
        bool done = true;
        for (int i=0; i<n_sensor; ++i) {
            if (idx >= offsets[i]) {
                Mat frame;
                if (caps[i].read(frame)) done = false;
                printf("%d %d\n", idx, i);
                sensors[i].next(idx, frame, time, server.getFeatures(i));
                server.clearBuf(i);
            }
        }
        if (done) break;
        idx++;
        time += MINI_SEC_PER_FRAME;
    }

    for (int i=0; i<n_sensor; ++i) {
        sensors[i].finish();
        senders[i]->finish();
    }
}
