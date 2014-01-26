#include "common/util.hpp"
#include "database/Dataset.hpp"

#include <opencv2/opencv.hpp>
using namespace cv;

#include <stdio.h>
#include <vector>
using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("usage: %s <dataset> <keyframe_list> <output_prefix>\n", argv[0]);
    return -1;
  }
  Dataset set("set", argv[1]);
  vector<VideoCapture> cap;
  vector<VideoInfo> & video = set.getVideoInfo();
  for (size_t i = 0; i<video.size(); ++i) {
    cap.push_back(VideoCapture(video[i].path));
  }

  FILE* ifile = fopen(argv[2], "r");
  vector<Keyframe> keyframe;
  parseKeyframe(ifile, keyframe);
  fclose(ifile);

  for (size_t i=0; i<keyframe.size(); ++i) {
    cap[keyframe[i].video_id].set(CV_CAP_PROP_POS_FRAMES, keyframe[i].frame_id);
    Mat frame;
    cap[keyframe[i].video_id].read(frame);
    char buf[128];
    sprintf(buf, "%s/%d.jpg", argv[3], (int) i);
    imwrite(buf, frame);
  }

}
