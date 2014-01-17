#ifndef SEGMENT_HPP
#define SEGMENT_HPP

struct Segment
{
  int start;
  int end;
  int video_id;
  Segment(int _video_id, int _start, int _end):
      start(_start), end(_end), video_id(_video_id) {}
};

#endif
