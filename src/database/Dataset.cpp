#include "database/dataset.hpp"

Dataset::Dataset(string name, string path):_name(name)
{
  string index_path = path + "/index.txt";
  FILE* ifile = fopen(index_path.c_str(), "r");
  if (ifile == NULL)
  {
    perror(string("Failed to open database index: " + name).c_str());
    return;
  }

  char buf[128];
  int offset;
  while (fscanf(ifile, "%s %d", buf, &offset) == 2)
  {
    _video.push_back(VideoInfo(path + '/' + buf, offset));
  }
}
