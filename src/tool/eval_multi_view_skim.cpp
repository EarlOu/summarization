#include "database/Dataset.hpp"
#include "common/Segment.hpp"
#include "common/util.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using std::vector;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage: %s <database> <skimming>\n", argv[0]);
    return -1;
  }

  Dataset set(argv[1]);
  vector<vector<Segment> > skim;
  FILE* ifile = fopen(argv[2], "r");
  parseMultiViewSkim(ifile, set.getVideoInfo(), skim);
  set.evaluateMultiViewSkim(skim);
  fclose(ifile);
}
