#include "database/Database.hpp"

#include <stdio.h>
#include <assert.h>

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
    string name(buf);
    _dataset.insert(
        pair<string, Dataset>(name, Dataset(name, dir_path + '/' + name)));
  }
}

Dataset& Database::getDataset(const string& name) {
  assert(_dataset.find(name) != _dataset.end());
  return _dataset[name];
}
