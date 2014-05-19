#ifndef DATASET_H
#define DATASET_H

#include "Event.h"
#include "common/VideoInfo.h"
#include "common/Keyframe.h"
#include "common/Segment.h"

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
    void evaluateSingleViewSkim(const vector<Segment>& skim, int video_id);
    void evaluateMultiViewSkim(const vector<vector<Segment> >& skim);
  private:
    vector<VideoInfo> _video;
    vector<Event> _event;
};

#endif
