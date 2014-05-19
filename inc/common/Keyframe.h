#ifndef KEYFRAME_H
#define KEYFRAME_H

struct Keyframe {
  int video_id;
  int frame_id;
  Keyframe(int vid, int fid): video_id(vid), frame_id(fid) {};
};
#endif
