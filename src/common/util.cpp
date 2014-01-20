#include "common/util.hpp"
#include <stdio.h>

void writeKeyframe(const vector<Keyframe>& keyframe, FILE* ofile) {
  for (size_t i = 0; i<keyframe.size(); ++i) {
    fprintf(ofile, "%d %d\n", keyframe[i].video_id, keyframe[i].frame_id);
  }
}

void parseKeyframe(FILE* ifile ,vector<Keyframe>& keyframe) {
  int vid, fid;
  while (fscanf(ifile, "%d %d", &vid, &fid) == 2) {
    keyframe.push_back(Keyframe(vid, fid));
  }
}
