#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "database/Dataset.hpp"

#include <string>
#include <map>

using namespace std;

class Database {
  public:
    Database(string path);
    Dataset& getDataset(const string& name);
  private:
    map<string, Dataset> _dataset;
};

#endif
