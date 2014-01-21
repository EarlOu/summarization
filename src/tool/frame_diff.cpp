#include <opencv2/opencv.hpp>
using namespace cv;

#include <stdio.h>

#define DIFF_TH 10
#define DIFF_RATE 0.005

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage: %s <video> <output.txt>\n", argv[0]);
    return -1;
  }
  VideoCapture cap(argv[1]);
  Mat rgb_frame, frame, prev;
  int start = 0;
  int idx = -1;
  FILE* ofile = fopen(argv[2], "w");
  while (cap.read(rgb_frame)) {
    ++idx;
    cvtColor(rgb_frame, frame, CV_BGR2GRAY);
    if (prev.empty()) {
      prev = frame.clone();
      continue;
    }
    Mat diff;
    absdiff(frame, prev, diff);
    float rate = countNonZero(diff > DIFF_TH) / (float) (frame.size().width * frame.size().height);
    if (rate > DIFF_RATE) {
      if (start == 0) {
        start = idx;
      }
    } else {
      if (start != 0) {
        fprintf(ofile, "%d %d\n", start, idx);
        start = 0;
      }
    }
    prev = frame.clone();
  }
  if (start != 0) {
    fprintf(ofile, "%d %d\n", start, idx);
    start = 0;
  }
  fclose(ofile);
}
