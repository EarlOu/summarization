#ifndef UTIL_HPP
#define UTIL_HPP

#include "common/Keyframe.hpp"

#include <vector>
using namespace std;

#include <stdio.h>

void writeKeyframe(const vector<Keyframe>& keyframe, FILE* ofile);
void parseKeyframe(FILE* ifile ,vector<Keyframe>& keyframe);

#endif
