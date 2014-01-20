#include "database/Database.hpp"

#include <stdio.h>
#include <assert.h>

Database::Database(string path)
{
  FILE* ifile = fopen(path.c_str(), "r");
  if (ifile == NULL)
  {
    perror("Failed to open database index");
    return;
  }
  string dir_path = path.substr(0, path.rfind('/'));
  char buf[128];
  while (fscanf(ifile, "%s", buf) == 1)
  {
    string name(buf);
    _dataset.insert(
        pair<string, Dataset>(name, Dataset(name, dir_path + '/' + name)));
  }
}

Dataset& Database::getDataset(const string& name)
{
  assert(_dataset.find(name) != _dataset.end());
  return _dataset[name];
}


void Dataset::evaluateKeyframe(const vector<Keyframe>& keyframe)
{
  int tp = 0;
  int redundant = 0;
  int event_get = 0;
  vector<bool> event_hit;
  for (size_t e=0; e<_event.size(); ++e) event_hit.push_back(false);

  for (size_t k=0; k<keyframe.size(); ++k) {
    for (size_t e=0; e<_event.size(); ++e) {
      for (size_t s=0; s<_event[e].size(); ++s) {
        const Segment & seg = _event[e][s];
        const Keyframe & key = keyframe[k];
        if (seg.video_id != key.video_id) continue;
        if (key.frame_id >= seg.start && key.frame_id < seg.end) {
          if (event_hit[e]) redundant++;
          else ++event_get;
          event_hit[e] = true;
          ++tp;
          break;
        }
      }
    }
  }
  printf("precision: %d/%d(%f)\n", tp, (int) keyframe.size(), tp / (float) keyframe.size());
  printf("recall: %d/%d(%f)\n", event_get, (int) _event.size(), event_get / (float) _event.size());
  printf("redundant frame: %d/%d(%f)\n", redundant, (int) keyframe.size(), redundant / (float) keyframe.size());
}
