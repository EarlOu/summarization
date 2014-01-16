#ifndef VIDEO_INFO_H
#define VIDEO_INFO_H

#include <cstring>

class VideoInfo {
  public:
    VideoInfo(string path, int offset): _path(path), _offset(offset) {}
    inline int getOffset() {return _offset;}
    inline string getPath() {return _path;}

  private:
    string _path;
    int _offset;
};

#endif
