#ifndef VIDEO_INFO_H
#define VIDEO_INFO_H

#include <string>
using namespace std;

class VideoInfo {
  public:
    VideoInfo(string path, int offset): _path(path), _offset(offset) {}
  private:
    int _offset;
    string _path;
};

#endif
