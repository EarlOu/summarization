#include <opencv2/opencv.hpp>
using namespace cv;

#include <stdio.h>

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    printf("usage: %s <video> <event.txt>\n", argv[0]);
    return -1;
  }
  FILE* ifile = fopen(argv[2], "r");
  vector<int> start;
  vector<int> end;
  int s, e;
  while (fscanf(ifile, "%d %d", &s, &e) == 2)
  {
    start.push_back(s);
    end.push_back(e);
  }
  fclose(ifile);

  VideoCapture cap(argv[1]);
  const char* name = argv[1];
  namedWindow(name);

  for (int i=0, n=start.size(); i<n; ++i)
  {
    int cs = start[i];
    int ce = end[i];
    printf("Segment: %d %d\n", cs, ce);
    cap.set(CV_CAP_PROP_POS_FRAMES, cs);
    Mat frame;
    for (int j=cs; j<ce; ++j) {
      cap.read(frame);
      imshow(name, frame);
      printf("%d\n", j);
      char k = waitKey(5);
      if (k == ' ' || j == cs) k = waitKey(0);
      if (j == ce - 1 && i == n - 1) {
        printf("%s\n", "Last frame");
        k = waitKey(0);
      }
      switch (k)
      {
        case 'r':
          j = ce;
          i = i-1;
          break;
        case 'b':
          j = ce;
          i = (i-2) > -1 ? (i-2) : -1;
          break;
        case 'n':
          j = ce;
          if (i == n-1) --i;
          break;
        case 'q':
          return 0;
      }
    }
  }
}
