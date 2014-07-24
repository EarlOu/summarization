#include "rpi/common/Common.h"

#include <stdlib.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace cv;

#include <vector>
#include <queue>
using std::vector;
using std::queue;
using std::priority_queue;

struct Seg {
    uint32_t s_time;
    uint32_t e_time;
    uint32_t n_frm;
    int vid;
};

const int TH_TIME_GAP = 400;
const int TH_MIN_SHOT_FRM = FPS * 1;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("usage: %s <directory> <n_view> <output.avi>\n", argv[0]);
        return -1;
    }

    typedef queue<Seg*> SegQueue;
    auto comp = [] (SegQueue* q1, SegQueue* q2) {return q1->front()->s_time > q2->front()->s_time;};
    priority_queue<SegQueue*, vector<SegQueue*>, decltype(comp)> pri_que(comp);
    vector<VideoCapture> caps;

    int n_view = atoi(argv[2]);
    if (n_view <= 0) return 0;

    for (int i=0; i<n_view; ++i) {
        char buf[128];

        sprintf(buf, "%s/info-%d.txt", argv[1], i);
        FILE* ifile = fopen(buf, "r");
        if (!ifile) {
            perror("Failed to open file for read.");
            exit(EXIT_FAILURE);
        }

        sprintf(buf, "%s/video-%d.264", argv[1], i);
        VideoCapture cap(buf);
        if (!cap.isOpened()) {
            perror("Failed to open video file.");
            exit(EXIT_FAILURE);
        }
        caps.push_back(cap);

        SegQueue* que = new SegQueue;
        int32_t s_time, e_time, n_frm;
        while (fscanf(ifile, "%u %u %u", &s_time, &e_time, &n_frm) == 3) {
            Seg* s = new Seg;
            s->s_time = s_time;
            s->e_time = e_time;
            s->n_frm = n_frm;
            s->vid = i;
            que->push(s);
        }
        if (!que->empty()) pri_que.push(que);
        fclose(ifile);
    }

    int w = caps[0].get(CV_CAP_PROP_FRAME_WIDTH);
    int h = caps[0].get(CV_CAP_PROP_FRAME_HEIGHT);
    VideoWriter writer(argv[3], CV_FOURCC('M', 'P', 'E', 'G'), FPS,
            Size(w, h));

    while (!pri_que.empty()) {
        SegQueue* que = pri_que.top();
        pri_que.pop();
        SegQueue buf;
        int n_frm = 0;
        while (!que->empty()) {
            if (!buf.empty() && que->front()->s_time - buf.back()->e_time > TH_TIME_GAP) break;
            n_frm += que->front()->n_frm;
            buf.push(que->front());
            que->pop();
        }

        while (!buf.empty()) {
            Seg* s = buf.front();
            buf.pop();
            int n = s->n_frm;
            int vid = s->vid;
            VideoCapture& cap = caps[vid];
            for (int i=0; i<n; ++i) {
                Mat frame;
                cap.read(frame);
                if (n_frm > TH_MIN_SHOT_FRM) {
                    writer.write(frame);
                }
            }
            delete s;
        }
        if (que->empty()) {
            delete que;
        } else {
            pri_que.push(que);
        }
    }
}
