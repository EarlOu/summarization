#ifndef DATABASE_H
#define DATABASE_H

#include "database/Dataset.hpp"

#include <string>
#include <vector>

using namespace std;

class Database {
  public:
    Database(string path);
  private:
    vector<Dataset> _dataset;
};

#endif
