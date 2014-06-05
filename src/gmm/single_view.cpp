#include  <opencv2/opencv.hpp>
#include "gmm/OnlineClusterMog.h"
#include "algorithm/FeatureExtractor.h"
#include "algorithm/FeatureExtractorLch.h"
#include "algorithm/FeatureExtractorCl.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace cv;
using std::vector;

// #define SHOW_VIDEO
// #define VERBOSE
// #define BACKGROUND

void computeHist(Mat frame, Mat &hist);
void extractBlockHist(Mat block, Mat hist);

typedef struct {
  int start;
  int end;
} Segment;

int main(int argc, char *argv[]) {
    if (argc != 2)
    {
        printf("usage: %s <input_video>\n", argv[0]);
        return -1;
    }

    FeatureExtractorCl featureExtractor;

    OnlineClusterMog onlineCluster;

    VideoCapture cap(argv[1]);
    int width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    Mat frame;
    Mat back_hist;

    int index = 0;
    int totalFrame = cap.get(CV_CAP_PROP_FRAME_COUNT);

#ifdef SHOW_VIDEO
    namedWindow("image");
#endif

#ifdef BACKGROUND
    namedWindow("feature");
    namedWindow("foreground");
    namedWindow("background");
    namedWindow("mask");
#endif

    vector<Segment> segments;
    Segment lastSegment;
    lastSegment.start = -1;
    lastSegment.end = -1;
    VideoWriter writer("output.avi", CV_FOURCC('M', 'P', 'E', 'G'), 30, Size(width, height));
    while (index < totalFrame && cap.read(frame))
    {
        Mat feature;
        featureExtractor.extract(frame, feature);
        Mat residue;
        int c = onlineCluster.cluster(feature, residue);
        bool chose = !onlineCluster.isBackground(c);

#ifdef BACKGROUND
        Mat backFeature;
        onlineCluster.getBackground(backFeature);
        Mat backFeatureFrame(frame.size(), frame.type());
        Mat currFeatureFrame(frame.size(), frame.type());

        Mat mask(frame.size(), CV_8UC1);
        mask.setTo(0);
        int w_step = frame.size().width / 8;
        int h_step = frame.size().height / 8;
        for (int y=0; y<8; ++y)
        {
            for (int x=0; x<8; ++x)
            {
                int i = (y * 8 + x) * 3;
                Scalar a(backFeature.at<float>(i), backFeature.at<float>(i+1),
                    backFeature.at<float>(i+2));
                Scalar b(feature.at<float>(i), feature.at<float>(i+1),
                    feature.at<float>(i+2));
                Scalar d = a - b;
                double diff = d[0]*d[0] + d[1]*d[1] + d[2]*d[2];

                if (diff > 0.01) {
                    mask.rowRange(y * h_step, y * h_step + h_step).
                    colRange(x * w_step, x * w_step + w_step).setTo(255);
                }
                backFeatureFrame.rowRange(y * h_step, y * h_step + h_step).
                    colRange(x * w_step, x * w_step + w_step).setTo(a * 255);
                currFeatureFrame.rowRange(y * h_step, y * h_step + h_step).
                    colRange(x * w_step, x * w_step + w_step).setTo(b * 255);
            }
        }
        Mat foreground(frame.size(), frame.type());
        foreground.setTo(Scalar(0,0,0));
        frame.copyTo(foreground, mask);
        imshow("mask", mask);
        imshow("foreground", foreground);
        imshow("feature", currFeatureFrame);
        imshow("background", backFeatureFrame);
#endif

#ifdef SHOW_VIDEO
        imshow("image", frame);
#endif
        // printf("Frame %d/%d\n", index, totalFrame);
        // printf("%d\n", chose ? 1 : 0);
        if (chose)
        {
          writer.write(frame);
        }

        if (chose) {
          if (lastSegment.start == -1) {
            lastSegment.start = index;
          }
        } else {
          if (lastSegment.start != -1) {
            lastSegment.end = index;
            segments.push_back(lastSegment);
            lastSegment.start = -1;
            lastSegment.end = -1;
          }
        }
#ifdef VERBOSE
        printf("Desicion: %s\n", chose ? "choose" : "ignore");
        printf("Cluster: %d\n", c);
        printf("\n");
#endif
        ++index;

#ifdef SHOW_VIDEO
        short k = waitKey(10);
        if (k == ' ') waitKey();
#endif
  }
    if (lastSegment.start != -1) {
      lastSegment.end = index;
      segments.push_back(lastSegment);
    }

    for (int i=0, n=segments.size(); i<n; ++i) {
       printf("%d %d\n", segments[i].start, segments[i].end);
    }
    writer.release();
}
