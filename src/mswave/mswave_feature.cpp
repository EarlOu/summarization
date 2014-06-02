#include <opencv2/opencv.hpp>
using namespace cv;

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 3 && argc != 4) {
    printf("usage: %s <video> <output.txt> [offset]\n", argv[0]);
    return -1;
  }

  int offset = 0;
  if (argc == 4) {
    offset = atoi(argv[3]);
  }

  FILE* ofile = fopen(argv[2], "w");
  for (int i=0; i<offset; ++i) {
    for (int j=0; j<256; ++j) fprintf(ofile, "%d ", 0);
    fprintf(ofile, "\n");
  }

  VideoCapture cap(argv[1]);
  BackgroundSubtractorMOG2 mog;

  float h_step = 180.0f / 16;
  float s_step = 256.0f / 4;
  float v_step = 256.0f / 4;

  Mat frame;
  while (cap.read(frame)) {
    Mat mask;

    // for office1 only
    resize(frame, frame, Size(), 0.5, 0.5);

    // mog(frame, mask, 0.0007); // for bl2, lobby
    mog(frame, mask); // for office 1
    mask = mask == 255;

    Mat hsv;
    cvtColor(frame, hsv, CV_BGR2HSV);
    Mat feature(256, 1, CV_32FC1);
    feature.setTo(0);
    if (countNonZero(mask) >
        frame.size().height * frame.size().width * 0.02) {
      for (int y=0; y<hsv.size().height; ++y) {
        uchar* ptr = hsv.ptr<uchar>(y);
        for (int x=0; x<hsv.size().width; ++x, ptr+=3) {
          if (mask.at<uchar>(y, x) == 0) continue;
          int h = floor(ptr[0] / h_step);
          int s = floor(ptr[1] / s_step);
          int v = floor(ptr[2] / v_step);
          int i = h + (s << 4) + (v << 6);
          feature.at<float>(i)++;
        }
      }
      normalize(feature, feature, 1, 0, NORM_L1);
      for (int i=0; i<256; ++i) {
        fprintf(ofile, "%f ", feature.at<float>(i));
      }
      fprintf(ofile, "\n");
    } else {
      for (int i=0; i<256; ++i) {
        fprintf(ofile, "%d ", 0);
      }
      fprintf(ofile, "\n");
    }
  }
  fclose(ofile);
}
