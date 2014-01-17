#ifndef VIDEO_INFO_HPP
#define VIDEO_INFO_HPP

#include <string>
using namespace std;

struct VideoInfo {
    const int offset;
    const string path;

    VideoInfo(const string& _path, int _offset):
        path(_path), offset(_offset) {}
};

#endif
