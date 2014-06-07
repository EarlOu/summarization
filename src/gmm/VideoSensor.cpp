#include "gmm/VideoSensor.h"
#include <stdlib.h>

//#define PACKAGE_LOST
//#define PACKAGE_LOST_RATE 0.3

#define N_BLOCK 8

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

bool VideoSensor::next(OutputArray oFrame, OutputArray oFeature, double& score)
{
    bool result = false;
    score = 0;
    if (_index >= _offset && _index < _offset + _length)
    {
        Mat frame;
        _cap.read(frame);
        oFrame.create(frame.size(), frame.type());
        frame.copyTo(oFrame.getMat());

        Mat feature;
        _extractor.extract(frame, feature);

        Mat residue;
        int cluster = _onlineCluster.cluster(feature, residue);
        if (!_onlineCluster.isBackground(cluster))
        {
            result = true;
        }
        Mat backFeature;
        _onlineCluster.getBackground(backFeature);

        extractIntraStageFeature(
                frame, feature, backFeature,
                oFeature, score);
        _feature = oFeature.getMat().clone();
        _score = score;
    }
    _index++;
    return result;
}

bool VideoSensor::receiveFeature(vector<Mat>& features, vector<double>& scores)
{
    for (int i=0, n=features.size(); i<n; ++i)
    {
#ifdef PACKAGE_LOST
        if (double(rand() % 100)/100.0 < PACKAGE_LOST_RATE) continue;
#endif
        double dist = compareHist(features[i], _feature, CV_COMP_BHATTACHARYYA);
        if (dist < 0.7 && scores[i] > _score) return false;
    }
    return true;
}

void VideoSensor::write(VideoWriter& writer, Segment& s)
{
    _cap.set(CV_CAP_PROP_POS_FRAMES, s.start - _offset);
    for (int i=0, l=s.end - s.start; i<l; ++i)
    {
        Mat frame;
        _cap.read(frame);
        writer.write(frame);
    }
}
