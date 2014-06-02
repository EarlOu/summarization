#include "database/Dataset.h"

#include <stdio.h>
#include <deque>
using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("usage: %s <dataset_dir> <score_dir> <output.txt>\n", argv[0]);
    return -1;
  }

  Dataset set(argv[1]);
  vector<VideoInfo> & info = set.getVideoInfo();
  FILE* ofile = fopen(argv[3], "w");

  //double threshold = 0.56; // bl7f
  //double threshold = 0.35; // office
  double threshold = 0.52; // lobby
  int win = 31;

  for (size_t vid=0; vid<info.size(); ++vid) {
    char buf[128];
    sprintf(buf, "%s/%d.txt", argv[2], (int) vid);
    FILE* ifile = fopen(buf, "r");

    double score = 0.0;
    int idx = 0;
    deque<double> scores(win);
    for (int i=0; i<win; ++i) {
      scores[i] = 0;
    }
    while (fscanf(ifile, "%lf", &score) == 1) {
      scores.pop_front();
      scores.push_back(score);

      if (scores[win/2] > threshold) {
        bool peak = true;
        for (int i=0; i<win; ++i) {
          if (scores[win/2] < scores[i]) {
            peak = false;
            break;
          }
        }
        if (peak && idx > win/2) {
          fprintf(ofile, "%d %d\n", (int) vid, idx - win/2 + info[vid].offset);
        }
      }
      idx++;
    }
    fclose(ifile);
  }
  fclose(ofile);
}
