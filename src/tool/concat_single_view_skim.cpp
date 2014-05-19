#include "database/Dataset.h"
#include "common/Segment.h"
#include "common/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using std::vector;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage: %s <database> <skimming_dir>\n", argv[0]);
    return -1;
  }

  Dataset set(argv[1]);
  vector<vector<Segment> > skim;
  parseMultiViewSkimFromDir(argv[2], set.getVideoInfo(), skim);
  for (int i=0, n=skim.size(); i<n; ++i) {
    for (int j=0, m=skim[i].size(); j<m; ++j) {
      printf("%d %d %d\n", skim[i][j].video_id, skim[i][j].start, skim[i][j].end);
    }
  }
}
