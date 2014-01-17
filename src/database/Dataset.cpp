#include "database/dataset.hpp"

Dataset::Dataset(const string& name, const string& path):_name(name)
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

  fclose(ifile);

  string event_path = path + "/event.txt";
  ifile = fopen(event_path.c_str(), "r");
  if (ifile == NULL) return;

  Event e;
  while (fgets(buf, 128, ifile))
  {
    if (buf[0] == '\n') // Empty line or EOF
    {
      if (e.size() != 0)
      {
        _event.push_back(e);
        e.clear();
      }
    } else if (buf[0] == '#') { // Comment
    } else {
      int id, start, end;
      sscanf(buf, "%d %d %d", &id, &start, &end);
      e.push_back(Segment(id, start, end));
    }
  }
  if (e.size() != 0)
  {
    _event.push_back(e);
    e.clear();
  }
}
