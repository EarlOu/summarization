#include "database/Database.hpp"

#include <stdio.h>

Database::Database(string path)
{
  FILE* ifile = fopen(path.c_str(), "r");
  if (ifile == NULL)
  {
    perror("Failed to open database index");
    return;
  }
  string dir_path = path.substr(0, path.rfind('/'));
  char buf[128];
  while (fscanf(ifile, "%s", buf) == 1)
  {
    string p = dir_path + '/' + buf;
    _dataset.push_back(Dataset(buf, p));
  }
}
