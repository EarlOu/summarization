#include "common/util.h"
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

void parseSingleViewSkim(FILE* ifile, int vid, int offset, vector<Segment>& skim) {
    int start, end;
    while (fscanf(ifile, "%d %d", &start, &end) == 2) {
        skim.push_back(Segment(vid, start + offset, end + offset));
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
        parseSingleViewSkim(ifile, i, info[i].offset, segments);
        skim.push_back(segments);
        fclose(ifile);
    }
}
