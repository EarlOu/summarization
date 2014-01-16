#ifndef DATASET_H
#define DATASET_H

#include "common/VideoInfo.hpp"

#include <string>
#include <vector>

using namespace std;

class Dataset {
  public:
    Dataset() {};
    Dataset(const string& name, const string& path);
    const string& getName() {return _name;}
    vector<VideoInfo>& getVideoInfo() {return _video;}
  private:
    const string _name;
    vector<VideoInfo> _video;
};

#endif
