#ifndef KEYFRAME_HPP
#define KEYFRAME_HPP

struct Keyframe {
  int video_id;
  int frame_id;
  Keyframe(int vid, int fid): video_id(vid), frame_id(fid) {};
};
#endif
