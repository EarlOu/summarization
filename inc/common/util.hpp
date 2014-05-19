#ifndef UTIL_HPP
#define UTIL_HPP

#include "common/Keyframe.hpp"
#include "common/Segment.hpp"
#include "common/VideoInfo.hpp"

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
