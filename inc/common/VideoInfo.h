#ifndef VIDEO_INFO_H
#define VIDEO_INFO_H

#include <string>
using namespace std;

struct VideoInfo {
    int offset;
    string path;

    VideoInfo(const string& _path, int _offset):
        path(_path), offset(_offset) {}
};

#endif
