#ifndef DATASET_H
#define DATASET_H

#include "common/VideoInfo.hpp"

#include <string>
#include <vector>

using namespace std;

class Dataset {
  public:
    Dataset(string name, string path);
  private:
    string _name;
    vector<VideoInfo> _video;
};

#endif
