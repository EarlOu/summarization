#include "database/dataset.h"
#include <assert.h>
#include <algorithm>
using std::sort;

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

bool sortByStart(const Segment& a, const Segment& b) {
  return a.start < b.start;
}

void Dataset::evaluateSingleViewSkim(const vector<Segment>& skim,
    int video_id) {
  vector<Segment> gt;
  for (int i=0, n=_event.size(); i<n; ++i) {
    for (int j=0, m=_event[i].size(); j<m; ++j) {
      if (_event[i][j].video_id == video_id) {
        gt.push_back(_event[i][j]);
      }
    }
  }

  // compute the tp
  int tp = 0;
  for (int i=0, n=skim.size(); i<n; ++i) {
    vector<Segment> segments;
    segments.push_back(
        Segment(skim[i].video_id, skim[i].start, skim[i].end));
    for (int j=0, m=gt.size(); j<m; ++j) {
      int gt_start = gt[j].start;
      int gt_end = gt[j].end;
      vector<int> remove_list;
      for (int k=0, l=segments.size(); k<l; ++k) {
        if (gt_start <= segments[k].start) {
          if (gt_end >= segments[k].end) {
            tp += segments[k].end - segments[k].start;
            remove_list.push_back(k);
          } else if (gt_end > segments[k].start) {
            tp += gt_end - segments[k].start;
            segments[k].start = gt_end;
          }
        } else if (gt_start < segments[k].end) {
          if (gt_end >= segments[k].end) {
            tp += segments[k].end - gt_start;
            segments[k].end = gt_start;
          } else {
            tp += gt_end - gt_start;
            segments.push_back(
                Segment(segments[k].video_id, gt_end, segments[k].end));
            segments[k].end = gt_start;
          }
        }
      }
      for (int k=remove_list.size()-1; k>=0; --k) {
        segments.erase(segments.begin() + remove_list[k]);
      }
    }
  }
  int gt_length = 0;
  int last = 0;
  vector<Segment> gt_sort = gt;
  sort(gt_sort.begin(), gt_sort.end(), sortByStart);
  for (int i=0, n=gt_sort.size(); i<n; ++i) {
    int l = gt_sort[i].end - ((gt_sort[i].start > last) ? gt_sort[i].start : last);
    if (l > 0) gt_length += l;
    last = gt_sort[i].end;
  }

  int summary_length = 0;
  for (int i=0, n=skim.size(); i<n; ++i) {
    summary_length += skim[i].end - skim[i].start;
  }

  printf("precision: %d/%d\n", tp, summary_length);
  printf("recall: %d/%d\n", tp, gt_length);

  int event_get = 0;
  int num_event = gt.size();
  for (int i=0, n=gt.size(); i<n; ++i) {
    int cover = 0;
    int length = gt[i].end - gt[i].start;
    for (int j=0, m=skim.size(); j<m; ++j) {
      int start = skim[j].start > gt[i].start ? skim[j].start : gt[i].start;
      int end = skim[j].end < gt[i].end ? skim[j].end : gt[i].end;
      if (end > start) cover += end - start;
      if (cover > length / 2) {
        event_get++;
        break;
      }
    }
  }
  printf("event recall: %d/%d\n", event_get, num_event);
}

void Dataset::evaluateMultiViewSkim(const vector<vector<Segment> >& skim) {
  int length = 0;
  for (int i=0, n=skim.size(); i<n; ++i) {
    for (int j=0, m=skim[i].size(); j<m; ++j) {
      length = skim[i][j].end > length ? skim[i][j].end : length;
    }
  }

  vector<int> skimIdx(skim.size(), 0);
  int tp = 0;
  int sumLength = 0;
  int gtLength = 0;
  for (int i=0; i<length; ++i) {
    vector<bool> getEvent(_event.size(), false);
    for (int j=0, n=skim.size(); j<n; ++j) {
      while (skimIdx[j] < skim[j].size() && i >= skim[j][skimIdx[j]].end) {
        skimIdx[j]++;
      }
      if (skimIdx[j] >= skim[j].size()) continue;
      bool getFrame = i >= skim[j][skimIdx[j]].start;
      if (getFrame) {
        bool tpAdd = false;
        for (int e=0, m=_event.size(); e<m; ++e) {
          if (getEvent[e]) continue;
          for (int k=0, l=_event[e].size(); k<l; ++k) {
            if (_event[e][k].video_id != j) continue;
            if (_event[e][k].start <= i && _event[e][k].end > i) {
              getEvent[e] = true;
              if (!tpAdd) {
                tp++;
                tpAdd = true;
              }
            }
          }
        }
        sumLength++;
      }
    }
    for (int e=0, m=_event.size(); e<m; ++e) {
      vector<bool> viewUsed(skim.size(), false);
      for (int k=0, l=_event[e].size(); k<l; ++k) {
        if (!viewUsed[_event[e][k].video_id] && _event[e][k].start <= i && _event[e][k].end > i) {
          gtLength++;
          viewUsed[_event[e][k].video_id] = true;
          break;
        }
      }
    }
  }
  printf("precision: %d/%d\n", tp, sumLength);
  printf("recall: %d/%d\n", tp, gtLength);
}
