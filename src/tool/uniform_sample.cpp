#include "database/Dataset.hpp"
#include "common/Keyframe.hpp"
#include "common/util.hpp"

#include <opencv2/opencv.hpp>
using namespace cv;

#include <vector>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("usage: %s <dataset> <k> <output.txt>\n", argv[0]);
    return -1;
  }
  Dataset set(argv[1]);
  const vector<VideoInfo> & video = set.getVideoInfo();

  vector<int> len;
  int max_len = 0;
  int k= atoi(argv[2]);

  for (size_t i=0; i<video.size(); ++i) {
    VideoCapture cap(video[i].path);
    int l = cap.get(CV_CAP_PROP_FRAME_COUNT);
    len.push_back(l);
    max_len = l > max_len ? l : max_len;
  }

  vector<Keyframe> keyframe;
  int step = max_len / k;
  int vid = 0;
  int fid = 0;
  while (fid < max_len) {
    while (video[vid].offset > fid || fid - video[vid].offset >= len[vid]) {
      vid++;
      if (vid >= len.size()) vid = 0;
    }

    keyframe.push_back(Keyframe(vid, fid));

    vid++;
    if (vid >= len.size()) vid = 0;
    fid += step;
  }

  FILE* ofile = fopen(argv[3], "w");
  writeKeyframe(keyframe, ofile);
  fclose(ofile);
}
