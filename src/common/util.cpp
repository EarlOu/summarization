#include "common/util.hpp"
#include <stdio.h>

void writeKeyframe(const vector<Keyframe>& keyframe, FILE* ofile) {
  for (size_t i = 0; i<keyframe.size(); ++i) {
    fprintf(ofile, "%d %d\n", keyframe[i].video_id, keyframe[i].frame_id);
  }
}

void parseKeyframe(FILE* ifile ,vector<Keyframe>& keyframe) {
  int vid, fid;
  while (fscanf(ifile, "%d %d", &vid, &fid) == 2) {
    keyframe.push_back(Keyframe(vid, fid));
  }
}

void parseSingleViewSkim(FILE* ifile, int offset, vector<Segment>& skim) {
//  int vid, start, end;
//  while (fscanf(ifile, "%d %d %d", &vid, &start, &end) == 3) {
//    skim.push_back(Segment(vid, start, end));
//  }
  int index = offset;
  int chose;
  Segment lastSegment(0, -1, -1);
  while (fscanf(ifile, "%d", &chose) == 1) {
    if (chose) {
      if (lastSegment.start == -1) {
        lastSegment.start = index;
      }
    } else {
      if (lastSegment.start != -1) {
        lastSegment.end = index;
        skim.push_back(lastSegment);
        lastSegment.start = -1;
        lastSegment.end = -1;
      }
    }
    ++index;
  }
  if (lastSegment.start != -1) {
    lastSegment.end = index;
    skim.push_back(lastSegment);
  }
}

void parseMultiViewSkim(FILE* ifile, const vector<VideoInfo>& info,
    vector<vector<Segment> >& skim) {
  int vid, start, end;
  skim = vector<vector<Segment> >(info.size());
  while (fscanf(ifile, "%d %d %d", &vid, &start, &end) == 3) {
    skim[vid].push_back(Segment(vid, start, end));
  }
}

void parseMultiViewSkimFromDir(string dirname, const vector<VideoInfo>& info,
    vector<vector<Segment> >& skim) {
  for (int i=0, n=info.size(); i<n; ++i) {
    char buf[128];
    sprintf(buf, "%s/%d.txt", dirname.c_str(), i);
    FILE* ifile = fopen(buf, "r");
    if (!ifile) {
      perror("Failed to open file.");
    }
    vector<Segment> segments;
    parseSingleViewSkim(ifile, info[i].offset, segments);
    for (int j=0, m=segments.size(); j<m; ++j) {
      segments[j].video_id = i;
    }
    skim.push_back(segments);
    fclose(ifile);
  }
}
