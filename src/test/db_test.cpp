#include "database/Dataset.hpp"

#include <stdio.h>
#include <vector>
using namespace std;

int main(int argc, char *argv[])
{
  Dataset set("name", argv[1]);
  vector<VideoInfo>& video = set.getVideoInfo();
  vector<Event>& event = set.getEvent();

  for (int i=0, n=event.size(); i<n; ++i) {
    printf("Event %d\n", i);
    for (int j=0, m=event[i].size(); j<m; ++j) {
      printf("%d %d %d\n", event[i][j].video_id, event[i][j].start, event[i][j].end);
    }
  }

  return 0;
}
