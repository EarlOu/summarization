#include "database/Database.hpp"

#include <stdio.h>
#include <vector>
using namespace std;

int main(int argc, char *argv[])
{
  Database db(argv[1]);
  Dataset& set = db.getDataset("bl2");
  vector<VideoInfo>& video = set.getVideoInfo();

  for (int i=0, n=video.size(); i<n; ++i)
  {
    printf("%s %d\n", video[i].path.c_str(), video[i].offset);
  }
  return 0;
}
