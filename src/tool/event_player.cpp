#include "common/VideoInfo.hpp"
#include "database/Event.hpp"
#include "database/Dataset.hpp"
#include "common/Segment.hpp"

#include <opencv2/opencv.hpp>
using namespace cv;

#include <vector>
using namespace std;

int main(int argc, char *argv[])
{

  if (argc != 2)
  {
    printf("usage: %s <dataset path>\n", argv[0]);
    return -1;
  }

  Dataset set("dataset", argv[1]);
  vector<VideoInfo> & video = set.getVideoInfo();
  vector<Event> & event = set.getEvent();
  vector<VideoCapture> cap;

  namedWindow("video");

  for (int i=0, n=video.size(); i<n; ++i)
  {
    cap.push_back(video[i].path);
  }

  for (int i=0, n=event.size(); i<n; ++i)
  {
    Event & e = event[i];
    for (int j=0, m=e.size(); j<m; ++j)
    {
      Segment seg = e[j];
      cap[seg.video_id].set(CV_CAP_PROP_POS_FRAMES, seg.start);
      for (int k=seg.start; k<seg.end; ++k) {
        Mat frame;
        cap[seg.video_id].read(frame);
        imshow("video", frame);
        waitKey(5);
      }
    }

  }
}
