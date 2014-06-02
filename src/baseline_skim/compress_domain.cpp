#include <opencv2/opencv.hpp>
#include <math.h>
using namespace cv;

// #define SHOW_VIDEO

#define SHOT_ERROR 0.1
#define SUM_RATE 50

typedef struct {
    int start;
    int end;
} Shot;

void quant(InputArray iFrame, OutputArray oQuan) {
    Mat frame = iFrame.getMat();
    int w = frame.size().width;
    int h = frame.size().height;

    oQuan.create(h, w, CV_8U);
    Mat quan = oQuan.getMat();

    for (int y=0; y<h; ++y) {
        for (int x=0; x<w; ++x) {
            Vec3b c = frame.at<Vec3b>(y, x);
            int ih = (int) floor(c[0] / 5.625);
            int is = c[1] / 64;
            int iv = c[2] / 128;
            int idx = ih + (is << 5) + (iv << 7);
            quan.at<uchar>(y, x) = idx;
        }
    }
}

double shotScore(Mat features[]) {
    for (int i=0; i<4; ++i) {
        if (features[i].empty()) return 2; // return the largest value
    }

    double zncc12 = compareHist(features[1], features[2], CV_COMP_CORREL);
    double zncc01 = compareHist(features[0], features[1], CV_COMP_CORREL);
    double zncc23 = compareHist(features[2], features[3], CV_COMP_CORREL);
    return zncc12/zncc23 + zncc12/zncc01;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s <video>\n", argv[0]);
        return -1;
    }

    VideoCapture cap(argv[1]);
    if (!cap.isOpened()) {
        perror("Failed to open video file.\n");
        return -1;
    }

    Mat frame;
    int total = cap.get(CV_CAP_PROP_FRAME_COUNT);
    int w = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int h = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    int dc_w = (w % 8 == 0) ? (w / 8) : (w / 8 + 1);
    int dc_h = (h % 8 == 0) ? (h / 8) : (h / 8 + 1);

    Mat features[4];

#ifdef SHOW_VIDEO
    namedWindow("frame");
#endif

    Mat prevQframe;

    vector<Shot> shots;
    int shotStart = 0;
    for (int i=0; i<total; ++i) {
        cap.read(frame);
        Mat dcFrame, qFrame;
        resize(frame, dcFrame, Size(dc_w, dc_h), 0, 0, INTER_AREA);
        cvtColor(dcFrame, dcFrame, CV_BGR2HSV);
        quant(dcFrame, qFrame);

        Mat hist;

        int channels[] = {0};
        int histSize[] = {256};
        float range[] = {0, 256};
        const float* ranges[] = {range};
        calcHist(&qFrame, 1, channels, Mat(), hist, 1, histSize, ranges);

        features[3] = features[2];
        features[2] = features[1];
        features[1] = features[0];
        features[0] = hist.clone();

        double s = shotScore(features);
        if (s < 2 - 2 * SHOT_ERROR) {
            if (i - shotStart > 15) {
                Shot shot;
                shot.start = shotStart;
                shot.end = i;
                shots.push_back(shot);
                shotStart = i;
            }
        }
#ifdef SHOW_VIDEO
        imshow("frame", frame);
        waitKey(5);
        if (s < 2 - 2 * SHOT_ERROR) waitKey(0);
#endif
    }

    if (total - shotStart > 15) {
        Shot shot;
        shot.start = shotStart;
        shot.end = total;
        shots.push_back(shot);
    }

    vector<Mat> selectedFrame;

    vector<Shot> result;
    VideoWriter writer("output.avi", CV_FOURCC('M', 'P', 'E', 'G'), 30, Size(w, h));
    for (int i=0, n=shots.size(); i<n; ++i) {
        Shot s = shots[i];
        for (int j=s.start; j<s.end; j+=100) {
            cap.set(CV_CAP_PROP_POS_FRAMES, j);
            vector<Mat> qFrames;
            int sim = 0;
            for (int k=j; k<j+SUM_RATE && k<s.end; ++k) {
                Mat frame;
                cap.read(frame);
                Mat dcFrame, qFrame;
                resize(frame, dcFrame, Size(dc_w, dc_h), 0, 0, INTER_AREA);
                cvtColor(dcFrame, dcFrame, CV_BGR2HSV);
                quant(dcFrame, qFrame);
                qFrames.push_back(qFrame);

                for (int l=0, m=selectedFrame.size(); l<m; ++l) {
                    Mat diff;
                    absdiff(selectedFrame[l], qFrame, diff);
                    int diffCount = countNonZero(diff > 5);
                    if (diffCount < 0.05 * dc_w * dc_h) {
                        sim++;
                        break;
                    }
                }
            }

            if (sim > SUM_RATE / 2) continue;
            for (int k=j; k<j+SUM_RATE && k<s.end; ++k) {
                Mat frame;
                cap.read(frame);
                writer.write(frame);
                selectedFrame.push_back(qFrames[k-j]);
            }
            Shot ss;
            ss.start = j;
            ss.end = j+SUM_RATE;
            if (ss.end > s.end) ss.end = s.end;
            result.push_back(ss);
        }
    }

    int j = 0;
    for (int i=0; i<total; ++i) {
        bool selected = false;
        if (i >= result[j].start) {
            selected = true;
        }
        if (i >= result[j].end) {
            selected = false;
            if (j < result.size() - 1) j++;
        }
        printf("%d\n", selected ? 1 : 0);
    }

}
