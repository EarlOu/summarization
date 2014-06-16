#include "gmm/Sensor.h"
#include "gmm/Sender.h"
#include "database/Dataset.h"
#include "common/util.h"

#include <stdio.h>
#include <assert.h>
#include <algorithm>
#include <opencv2/opencv.hpp>
using namespace cv;

#define FPS 30
#define MINI_SEC_PER_FRAME 33

class Server {
private:
    typedef vector<Segment> MergedSkim;
public:
    Server(int n_sensor, FILE* ofile): _n_sensor(n_sensor), _ofile(ofile),
            _features(n_sensor),
            _skim_start(n_sensor, false),
            _skim_start_idx(n_sensor, 0),
            _last_received_idx(n_sensor, 0),
            _skim(n_sensor) {}

    void broadcastFeature(InputArray iFeature, double score, size_t time, int vid) {
        for (int i=0; i<_n_sensor; ++i) {
            if (i == vid) continue;
            _features[i].push_back(FeaturePacket(iFeature.getMat(), score, time));
        }
    }

    list<FeaturePacket>& getFeatures(int vid) {
        return _features[vid];
    }

    void clearBuf(int vid) {
        _features[vid].clear();
    }

    void sendFrame(int idx, int vid) {
        if (_skim_start[vid]) {
            if (idx != _last_received_idx[vid] + 1) { // the shot is break
                fprintf(_ofile, "%d %d %d\n", vid, _skim_start_idx[vid], _last_received_idx[vid] + 1);
                _skim[vid].push_back(Segment(vid, _skim_start_idx[vid], _last_received_idx[vid] + 1));
                _skim_start_idx[vid] = idx;
            }
        } else {
            _skim_start[vid] = true;
            _skim_start_idx[vid] = idx;
        }
        _last_received_idx[vid] = idx;
    }

    void finish() {
        for (int i=0; i<_n_sensor; ++i) {
            if (_skim_start[i]) {
                fprintf(_ofile, "%d %d %d\n", i, _skim_start_idx[i], _last_received_idx[i] + 1);
                _skim[i].push_back(Segment(i, _skim_start_idx[i], _last_received_idx[i] + 1));
            }
        }
        postProcess();
    }

    static bool compareSkim(MergedSkim* s1, MergedSkim* s2) {
        return s1->front().start < s2->front().start;
    }

    void postProcess() {
        for (int i=0; i<_n_sensor; ++i) {
            MergedSkim* ms = new MergedSkim;
            for (int j=0, n=_skim[i].size(); j<n; ++j) {
                if (ms->size() == 0 || _skim[i][j].start - ms->back().end < FPS) {
                    ms->push_back(_skim[i][j]);
                } else {
                    if (ms->back().end - ms->front().start >= FPS) {
                        _merged_skim.push_back(ms);
                    } else {
                        delete ms;
                    }
                    ms = new MergedSkim;
                    ms->push_back(_skim[i][j]);
                }
            }
            if (ms->size() != 0 && ms->back().end - ms->front().start >= FPS) {
                _merged_skim.push_back(ms);
            } else {
                delete ms;
            }
        }

        std::sort(_merged_skim.begin(), _merged_skim.end(), compareSkim);
    }

    void writeVideo(vector<VideoCapture*>& caps, vector<int>& offsets, VideoWriter& writer) {
        for (int i=0, n=_merged_skim.size(); i<n; ++i) {
            MergedSkim* ms = _merged_skim[i];
            for (int j=0, m=ms->size(); j<m; ++j) {
                int vid = ms->operator[](j).video_id;
                int start = ms->operator[](j).start;
                int end = ms->operator[](j).end;
                caps[vid]->set(CV_CAP_PROP_POS_FRAMES, start - offsets[vid]);
                for (int k=start; k<end; ++k) {
                    Mat frame;
                    assert(caps[vid]->read(frame));
                    writer.write(frame);
                }
            }
        }
    }

private:
    int _n_sensor;
    FILE* _ofile;
    vector<list<FeaturePacket> > _features;
    vector<bool> _skim_start;
    vector<int> _skim_start_idx;
    vector<int> _last_received_idx;
    vector<vector<Segment> > _skim;
    vector<MergedSkim* > _merged_skim;
};

class SimulateSender:public Sender {
public:
    SimulateSender(Server& server, int vid)
            :_server(server), _vid(vid) {}

    virtual void sendFrame(InputArray iFrame, size_t time, int idx) {
        _server.sendFrame(idx, _vid);
    }

    virtual void sendFeature(InputArray iFeature, double score, size_t time, int idx) {
        _server.broadcastFeature(iFeature, score, time, _vid);
    }

    virtual void finish() {}
private:
    Server & _server;
    int _vid;
};

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        printf("usage: %s <dataset_dir> <output.txt> [output.avi]\n", argv[0]);
        return -1;
    }

    Dataset set(argv[1]);
    int n_sensor = set.getVideoInfo().size();

    FILE* ofile = fopen(argv[2], "w");
    if (!ofile) {
        perror("Failed to open output file");
        return -1;
    }

    Server server(n_sensor, ofile);
    vector<VideoCapture*> caps;
    vector<int> offsets;
    vector<int> lengths;
    vector<SimulateSender*> senders;
    vector<Sensor*> sensors;

    for (int i=0; i<n_sensor; ++i) {
        VideoInfo info = set.getVideoInfo()[i];
        VideoCapture* cap = new VideoCapture(info.path);

        offsets.push_back(info.offset);
        lengths.push_back(info.offset + cap->get(CV_CAP_PROP_FRAME_COUNT));

        caps.push_back(cap);
        senders.push_back(new SimulateSender(server, i));
        sensors.push_back(new Sensor(senders[i]));
    }

    int idx = 0;
    size_t time = 0;
    while (true) {
        bool done = true;
        for (int i=0; i<n_sensor; ++i) {
            if (idx >= offsets[i]) {
                Mat frame;
                if (idx < lengths[i] && caps[i]->read(frame)) done = false;
                sensors[i]->next(idx, frame, time, server.getFeatures(i));
                server.clearBuf(i);
            }
        }
        if (done) break;
        idx++;
        time += MINI_SEC_PER_FRAME;
    }

    for (int i=0; i<n_sensor; ++i) {
        sensors[i]->finish();
        senders[i]->finish();
    }

    server.finish();

    int w = caps[0]->get(CV_CAP_PROP_FRAME_WIDTH);
    int h = caps[0]->get(CV_CAP_PROP_FRAME_HEIGHT);

    if (argc == 4) {
        VideoWriter writer(argv[3], CV_FOURCC('M', 'P', 'E', 'G'), FPS,
                Size(w, h));
        server.writeVideo(caps, offsets, writer);
    }
}
