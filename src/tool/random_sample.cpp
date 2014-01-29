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
  int k= atoi(argv[2]);

  for (size_t i=0; i<video.size(); ++i) {
    VideoCapture cap(video[i].path);
    len.push_back(cap.get(CV_CAP_PROP_FRAME_COUNT));
  }

  srand(time(NULL));
  vector<Keyframe> keyframe;

  for (int i=0; i<k; ++i) {
    int vid = rand() % len.size();
    int fid = rand() % len[vid];
    keyframe.push_back(Keyframe(vid, fid + video[vid].offset));
  }
  FILE* ofile = fopen(argv[3], "w");
  writeKeyframe(keyframe, ofile);
  fclose(ofile);
}
