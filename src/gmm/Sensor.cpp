#include "gmm/Sensor.h"
#include "gmm/OnlineClusterMog.h"
#include "algorithm/FeatureExtractorCl.h"

#include <stdlib.h>
#include <sys/time.h>
#include <opencv2/opencv.hpp>
using namespace cv;

#define N_BLOCK 8
#define BUFFER_TIME 3000 // 3 seconds buffer time for network delay
#define FEATURE_MATCH_TIME_TH 10
#define FEATURE_MATCH_TH 0.7

static void extractIntraStageFeature(
        InputArray iFrame, InputArray iFeature, InputArray iBackground,
        OutputArray oFeature, double & score) {

    Mat frame = iFrame.getMat();
    Mat feature = iFeature.getMat();
    Mat backFeature = iBackground.getMat();

    // create mask
    Mat mask(frame.size(), CV_8UC1);
    mask.setTo(0);
    int w_step = frame.size().width / N_BLOCK;
    int h_step = frame.size().height / N_BLOCK;
    int crop_w = w_step * N_BLOCK;
    int crop_h = h_step * N_BLOCK;
    int foreground_count = 0;
    int i = 0;
    for (int y=0; y<crop_h; y+=h_step) {
        for (int x=0; x<crop_w; x+=w_step) {
            Scalar a(backFeature.at<float>(i), backFeature.at<float>(i+1),
                    backFeature.at<float>(i+2));
            Scalar b(feature.at<float>(i), feature.at<float>(i+1),
                    feature.at<float>(i+2));
            Scalar d = a - b;
            double diff = d[0]*d[0] + d[1]*d[1] + d[2]*d[2];
            i += 3;

            if (diff > 0.01) {
                mask.rowRange(y, y + h_step).
                    colRange(x, x + w_step).setTo(255);
                foreground_count++;
            }
        }
    }

    // compute HSV histogram
    static const int histSize = 16;
    static const float range[] = {0, 181};
    static const float* histRange = {range};
    Mat hsvFrame;
    cvtColor(frame, hsvFrame, CV_BGR2HSV);
    vector<Mat> hsv;
    split(hsvFrame, hsv);
    Mat hFrame = hsv[0];
    calcHist(&hFrame, 1, 0, mask, oFeature, 1, &histSize, &histRange, true, false);

    // compute score
    Mat residue;
    absdiff(feature, backFeature, residue);
    Scalar s = sum(residue);
    score = s[0] + s[1] + s[2];
}

void Sensor::next(int idx, cv::InputArray iFrame, size_t time, list<ReceivedFeature>& features) {
    Mat frame = iFrame.getMat();

    // intra stage
    bool choose = false;
    Mat feature;
    Mat residue;
    if (!frame.empty()) {
        _featureExtractor.extract(frame, feature);
        int c = _onlineCluster.cluster(feature, residue);
        choose = !_onlineCluster.isBackground(c);
    }

    if (INTRA_ONLY) {
        if (choose) {
            _sender->sendFrame(frame, time, idx);
        }
        return;
    }

    //inter stage
    if (choose) {
        Mat background;
        Mat intraFeature;
        double score;
        _onlineCluster.getBackground(background);
        extractIntraStageFeature(frame, feature, background, intraFeature, score);
        _buf.push_back(BufferedFeature(intraFeature, score, time, frame, idx));
        _sender->sendFeature(intraFeature, score, time, idx);
    }

    // match frames in the buffer
    for (list<ReceivedFeature>::iterator it_r = features.begin(); it_r != features.end(); it_r++) {
        list<BufferedFeature>::iterator it_b;
        for (it_b = _buf.begin(); it_b != _buf.end(); it_b++) {
            if (abs(it_r->time - it_b->time) < FEATURE_MATCH_TIME_TH) break;
        }
        if (it_b != _buf.end()
                && it_r->score > it_b->score
                && compareHist(it_r->feature, it_b->feature, CV_COMP_BHATTACHARYYA) < FEATURE_MATCH_TH) {
            _buf.erase(it_b);
        }
    }

    // send no matching frames
    while (!_buf.empty() && time - _buf.front().time > BUFFER_TIME) {
        _sender->sendFrame(_buf.front().frame, _buf.front().time, _buf.front().idx);
        _buf.pop_front();
    }
    return;
}

void Sensor::finish() {
    if (!INTRA_ONLY) {
        // Send all remaining frames in the buffer
        for (list<BufferedFeature>::iterator it = _buf.begin(); it != _buf.end(); it++) {
            _sender->sendFrame(it->frame, it->time, it->idx);
        }
    }
}
