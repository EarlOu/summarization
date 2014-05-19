#ifndef UTIL_H
#define UTIL_H

#include "Keyframe.h"
#include "Segment.h"
#include "VideoInfo.h"

#include <vector>
#include <string>
using namespace std;

#include <stdio.h>

void writeKeyframe(const vector<Keyframe>& keyframe, FILE* ofile);
void parseKeyframe(FILE* ifile ,vector<Keyframe>& keyframe);
void parseSingleViewSkim(FILE* ifile, int offset, vector<Segment>& skim);
void parseMultiViewSkim(FILE* ifile, const vector<VideoInfo>& info,
    vector<vector<Segment> >& skim);
void parseMultiViewSkimFromDir(string dirname, const vector<VideoInfo>& info,
    vector<vector<Segment> >& skim);

#endif
