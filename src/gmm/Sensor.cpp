#include "gmm/Sensor.h"
#include "algorithm/FeatureExtractorCl.h"
#include "gmm/OnlineClusterMog.h"
#include <sys/time.h>
#include <opencv2/opencv.hpp>
using namespace cv;

#define N_BLOCK 8
#define BUFFER_TIME 3000 // 3 seconds buffer time for network delay

static inline size_t gettime() {
    static struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000L + tv.tv_usec / 1000L;
}

static void extractIntraStageFeature(
        InputArray iFrame, InputArray iFeature, InputArray iBackground,
        OutputArray oFeature, float & score) {

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

void Sensor::run() {
    Mat frame;
    FeatureExtractorCl featureExtractor(N_BLOCK);
    OnlineClusterMog onlineCluster;

    int idx = 0;
    size_t time;
    while (read(frame, time)) {
        if (frame.empty()) continue;

        // intra stage
        Mat feature;
        Mat residue;
        featureExtractor.extract(frame, feature);
        int c = onlineCluster.cluster(feature, residue);
        bool choose = !onlineCluster.isBackground(c);

        if (_inter_stage) {
            //inter stage
            if (choose) {
                Mat background;
                Mat intraFeature;
                float score;
                onlineCluster.getBackground(background);
                extractIntraStageFeature(frame, feature, background, intraFeature, score);
                _buf.push_back(BufferedFrame(frame, intraFeature, score, idx, time));
            }

            // vector<ReceivedFeature> features;
            // receiveFeature(features) {
            // }

            // send frame with no match
            for (list<BufferedFrame>::iterator it = _buf.begin(); it != _buf.end(); it++) {
                if (time - it->time > BUFFER_TIME) {
                    sendFrame(it->frame, it->idx);
                    _buf.erase(it);
                }
            }
        } else {
            if (choose) {
                sendFrame(frame, idx);
            }
        }
        frame = Mat();
        idx++;
    }

    // loop finish, send all remaining frames in the buffer
    for (list<BufferedFrame>::iterator it = _buf.begin(); it != _buf.end(); it++) {
        sendFrame(it->frame, it->idx);
    }
}
