#ifndef DATASET_HPP
#define DATASET_HPP

#include "database/Event.hpp"
#include "common/VideoInfo.hpp"
#include "common/Keyframe.hpp"

#include <string>
#include <vector>

using namespace std;

class Dataset {
  public:
    Dataset() {};
    Dataset(const string& path);
    vector<VideoInfo>& getVideoInfo() {return _video;}
    vector<Event>& getEvent() {return _event;}

    void evaluateKeyframe(const vector<Keyframe>& keyframe);
  private:
    vector<VideoInfo> _video;
    vector<Event> _event;
};

#endif
