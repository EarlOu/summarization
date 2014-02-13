#include "database/dataset.hpp"
#include <assert.h>

Dataset::Dataset(const string& path)
{
  string index_path = path + "/index.txt";
  FILE* ifile = fopen(index_path.c_str(), "r");
  if (ifile == NULL)
  {
    perror((string("Failed to open database index: ") + path).c_str());
    assert(false);
  }

  char buf[128];
  int offset;
  while (fscanf(ifile, "%s %d", buf, &offset) == 2)
  {
    _video.push_back(VideoInfo(path + '/' + buf, offset));
  }

  fclose(ifile);

  string event_path = path + "/event.txt";
  ifile = fopen(event_path.c_str(), "r");
  if (ifile == NULL) return;

  Event e;
  while (fgets(buf, 128, ifile))
  {
    if (buf[0] == '\n') // Empty line or EOF
    {
      if (e.size() != 0)
      {
        _event.push_back(e);
        e.clear();
      }
    } else if (buf[0] == '#') { // Comment
    } else {
      int id, start, end;
      sscanf(buf, "%d %d %d", &id, &start, &end);
      e.push_back(Segment(id, start, end));
    }
  }
  if (e.size() != 0)
  {
    _event.push_back(e);
    e.clear();
  }
}

void Dataset::evaluateKeyframe(const vector<Keyframe>& keyframe)
{
  int tp = 0;
  int redundant = 0;
  int event_get = 0;
  vector<bool> event_hit;
  for (size_t e=0; e<_event.size(); ++e) event_hit.push_back(false);

  bool isAdd = false;
  for (size_t k=0; k<keyframe.size(); ++k) {
    isAdd = false;
    for (size_t e=0; e<_event.size(); ++e) {
      for (size_t s=0; s<_event[e].size(); ++s) {
        const Segment & seg = _event[e][s];
        const Keyframe & key = keyframe[k];
        if (seg.video_id != key.video_id) continue;
        if (key.frame_id >= seg.start && key.frame_id < seg.end) {
          if (event_hit[e]) redundant++;
          else ++event_get;
          event_hit[e] = true;
          if (!isAdd) ++tp;
          isAdd = true;
          break;
        }
      }
    }
  }
  printf("precision: %d/%d(%f)\n", tp, (int) keyframe.size(), tp / (float) keyframe.size());
  printf("recall: %d/%d(%f)\n", event_get, (int) _event.size(), event_get / (float) _event.size());
  printf("redundant frame: %d/%d(%f)\n", redundant, (int) keyframe.size(), redundant / (float) keyframe.size());
}
