#include "database/Dataset.hpp"
#include "common/Keyframe.hpp"
#include "common/util.hpp"

#include <stdio.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage: %s <dataset> <keyframe.txt>\n", argv[0]);
    return -1;
  }
  Dataset set("dataset", argv[1]);
  FILE* ifile = fopen(argv[2], "r");
  vector<Keyframe> keyframe;
  parseKeyframe(ifile, keyframe);
  fclose(ifile);
  set.evaluateKeyframe(keyframe);
}
