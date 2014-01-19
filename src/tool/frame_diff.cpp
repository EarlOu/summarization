#include <opencv2/opencv.hpp>
using namespace cv;

#include <stdio.h>

#define DIFF_TH 10
#define DIFF_RATE 0.001

int main(int argc, char *argv[])
{
  VideoCapture cap(argv[1]);
  Mat rgb_frame, frame, prev;
  int start = 0;
  int idx = -1;
  while (cap.read(rgb_frame))
  {
    ++idx;
    cvtColor(rgb_frame, frame, CV_BGR2GRAY);
    if (prev.empty())
    {
      prev = frame.clone();
      continue;
    }
    Mat diff;
    absdiff(frame, prev, diff);
    float rate = countNonZero(diff > DIFF_TH) / (float) (frame.size().width * frame.size().height);
    if (rate > DIFF_RATE)
    {
      if (start == 0)
      {
        start = idx;
      }
    } else {
      if (start != 0)
      {
        printf("%d %d\n", start, idx);
        start = 0;
      }
    }
    prev = frame.clone();
  }
  if (start != 0)
  {
    printf("%d %d\n", start, idx);
    start = 0;
  }
}
