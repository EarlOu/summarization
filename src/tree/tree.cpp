#include <opencv2/opencv.hpp>
#include <deque>
#include <math.h>
#include <map>
#include <utility>

#include "algorithm/FeatureExtractorLch.h"
#include "algorithm/FeatureExtractorCl.h"

#define SHOT_LENGTH 30
#define SUMMARY_RATE 0.05
//#define SUMMARY_RATE 0.1
//#define D 30
#define D 90
#define PRUNE_SIZE 1000

using namespace cv;
using std::deque;

typedef vector<Mat> Shot;

bool readShot(VideoCapture& cap, vector<Mat>& shot);

class Node;

class Node {
public:
  int totalF;
  int includeF;
  int includeN;
  int continuity;
  double rScore;
  double aScore;
  double sizeScore;
  double continuityScore;
  bool select;
  Node* parent;
  Node* children[2];
  double meanDistance;
  double activity;
  int nF;
};

void setParent(Node* child, Node* parent)
{
  parent->children[child->select ? 1 : 0] = child;
  child->parent = parent;
}

void computeInfo(Node* child)
{
  Node* parent = child->parent;
  child->totalF = parent->totalF + child->nF;
  child->includeF = parent->includeF + (child->select ? child->nF : 0);
  child->includeN = parent->includeN + (child->select ? 1 : 0);
  child->continuity = parent->continuity +
      ((child->select && parent->select) ? 1 : 0);

  double parentR = parent->rScore * parent->includeF;
  double currentR = child->meanDistance * child->nF;
  child->rScore = (child->includeF == 0) ? 0 : ((parentR + currentR) / child->includeF);

  double parentA = parent->aScore * parent->includeF;
  double currentA = child->activity * child->nF;
  child->aScore = (child->includeF == 0) ? 0 : ((parentA + currentA) / child->includeF);

  double rate = double(child->includeF) / (child->totalF * SUMMARY_RATE);
  child->sizeScore = (rate > 1) ? (1 / rate) : sqrt(rate);

  child->continuityScore = (child->includeN == 0) ? 0 : (double(child->continuity) / child->includeN);

  // printf("Select: %d\n", child->select ?1 : 0);
  // printf("TotalF: %d\n", child->totalF);
  // printf("IncludeF: %d\n", child->includeF);
  // printf("IncludeN: %d\n", child->includeN);
  // printf("continuity: %d\n", child->continuity);
  // printf("rScore: %lf\n", child->rScore);
  // printf("aScore: %lf\n", child->aScore);
  // printf("sizeScore: %lf\n", child->sizeScore);
  // printf("continuityScore: %lf\n", child->continuityScore);
  // printf("meanDistance: %lf\n", child->meanDistance);
  // printf("activity: %lf\n", child->activity);
  // printf("\n");
}

double Distance(vector<Mat>& f1, vector<Mat>& f2)
{
  double diff = 0;
  for (int i=0, n=f1.size(); i<n; ++i)
  {
    for (int j=0, m=f2.size(); j<m; ++j)
    {
      diff += norm(f1[i], f2[j]);
    }
  }
  int c = f1.size() * f2.size();
  if (c != 0) diff /= c;
  return diff;
}

void computeMeanDistance(deque<double>& distances, Node* node)
{
  if (!node->select)
  {
    node->meanDistance = 0;
    return;
  }

  double diff = 0;
  Node* n = node;
  for (int i=distances.size() - 1; i>=0; --i)
  {
    n = n->parent;
    diff += n->select ? distances[i] : 0;
  }
  diff /= (node->parent->includeN + 1);
  node->meanDistance = diff;
}

double computeActivity(vector<Mat>& feature)
{
  double activity = 0;
  for (int i=0, n=feature.size() - 1; i<n; ++i)
  {
    activity += norm(feature[i], feature[i+1]);
  }
  if (feature.size() != 0) activity /= feature.size();
  return activity;
}

void extract(Shot& shot, vector<Mat>& feature)
{
  FeatureExtractorCl extractor;
  for (int i=0, n=shot.size(); i<n; ++i)
  {
    Mat f;
    extractor.extract(shot[i], f);
    feature.push_back(f);
  }
}

void findMinMax(vector<Node*> nodes, Node& max, Node& min)
{
  max.sizeScore = -INT_MAX;
  max.continuityScore = -INT_MAX;
  max.rScore = -INT_MAX;
  max.aScore = -INT_MAX;
  min.sizeScore = INT_MAX;
  min.continuityScore = INT_MAX;
  min.rScore = INT_MAX;
  min.aScore = INT_MAX;

  for (int i=0, m=nodes.size(); i<m; ++i)
  {
    Node* n = nodes[i];
    max.sizeScore = std::max(max.sizeScore, n->sizeScore);
    max.continuityScore = std::max(max.continuityScore, n->continuityScore);
    max.rScore = std::max(max.rScore, n->rScore);
    max.aScore = std::max(max.aScore, n->aScore);

    min.sizeScore = std::min(min.sizeScore, n->sizeScore);
    min.continuityScore = std::min(min.continuityScore, n->continuityScore);
    min.rScore = std::min(min.rScore, n->rScore);
    min.aScore = std::min(min.aScore, n->aScore);
  }
}

double computeScore(Node& node, Node& max, Node& min)
{
  double s = 0;
  if (max.sizeScore - min.sizeScore)
  {
    //s += 0.15 * (node.sizeScore - min.sizeScore) / (max.sizeScore - min.sizeScore);
    s += 0.2 * (node.sizeScore - min.sizeScore) / (max.sizeScore - min.sizeScore);
  }
  if (max.continuityScore - min.continuityScore)
  {
    s += 0 * (node.continuityScore - min.continuityScore) / (max.continuityScore - min.continuityScore);
  }
  if (max.rScore - min.rScore)
  {
    //s += 0.7 * (node.rScore - min.rScore) / (max.rScore - min.rScore);
    s += 0.6 * (node.rScore - min.rScore) / (max.rScore - min.rScore);
  }
  if (max.aScore - min.aScore)
  {
    //s += 0.15 * (node.aScore - min.aScore) / (max.aScore - min.aScore);
    s += 0.3 * (node.aScore - min.aScore) / (max.aScore - min.aScore);
  }
  return s;
}

class NodeCompare
{
public:
  NodeCompare(Node& max, Node& min): _max(max), _min(min) {}
  bool operator()(Node* n1, Node* n2)
  {
    return (computeScore(*n1, _max, _min) > computeScore(*n2, _max, _min));
  }
private:
  Node _max;
  Node _min;
};

void updateRoot(Node* newRoot, deque<deque<double> >& dists)
{
  newRoot->totalF = 0;
  newRoot->includeF = 0;
  newRoot->includeN = 0;
  newRoot->continuity = 0;
  newRoot->meanDistance = 0;
  newRoot->activity = 0;
  newRoot->rScore = 0;
  newRoot->aScore = 0;
  newRoot->sizeScore = 0;
  newRoot->continuityScore = 0;

  deque<Node*> q;
  q.push_back(newRoot->children[0]);
  q.push_back(newRoot->children[1]);

  int layer = 1;
  while (!q.empty())
  {
    deque<Node*> newQ;
    double dist = 0;
    double exN = 0;
    if (newRoot->select)
    {
      dist = dists[layer][0];
      exN = 1;
    }
    while (!q.empty())
    {
      Node* n = q.front();
      q.pop_front();
      if (n->children[0]) newQ.push_back(n->children[0]);
      if (n->children[1]) newQ.push_back(n->children[1]);

      Node* parent = n->parent;

      n->meanDistance = n->select ? (((n->meanDistance * n->includeN) - dist) / (n->includeN - exN)) : 0;
      n->totalF = parent->totalF + n->nF;
      n->includeF = parent->includeF + (n->select ? n->nF : 0);
      n->includeN = parent->includeN + (n->select ? 1 : 0);
      n->continuity = parent->continuity +
          ((parent->select && n->select) ? 1 : 0);

      n->rScore = (n->includeF == 0) ? 0 : ((parent->rScore * parent->includeF) + n->meanDistance * n->nF) / n->includeF;
      n->aScore = (n->includeF == 0) ? 0 : ((parent->aScore * parent->includeF) + n->activity * n->nF) / n->includeF;

      double rate = double(n->includeF) / (n->totalF * SUMMARY_RATE);
      n->sizeScore = (rate > 1) ? (1 / rate) : sqrt(rate);

      n->continuityScore = (n->includeN == 0) ? 0 : (double(n->continuity) / n->includeN);
    }
    layer++;
    q = newQ;
  }
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("usage: %s <video>\n", argv[0]);
    return -1;
  }
  VideoCapture cap(argv[1]);

  Mat frame;
  Shot curr_shot;
  deque<Shot> shots;
  deque<vector<Mat> > features;
  deque<deque<double> > distances;

  vector<Node*> leaf;

  Node* root = new Node;
  root->totalF = 0;
  root->includeF = 0;
  root->includeN = 0;
  root->continuity = 0;
  root->rScore = 0;
  root->aScore = 0;
  root->select = false;
  root->nF = 0;

  leaf.push_back(root);

  int width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
  int height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
  VideoWriter writer("output.avi", CV_FOURCC('M', 'P', 'E', 'G'), 30, Size(width, height));
  int index = 1;
  while (readShot(cap, curr_shot))
  {
    // printf("Read %d shot: %d\n", index++, (int) curr_shot.size());

    // Read a new shot and extract feature
    shots.push_back(curr_shot);
    vector<Mat> feature;
    extract(curr_shot, feature);
    features.push_back(feature);
    deque<double> dists;
    for (int i=0, n=features.size() - 1; i<n; ++i)
    {
      dists.push_back(Distance(features[i], feature));
    }
    distances.push_back(dists);
    double activity = computeActivity(feature);

    // Create a new level of leaves
    vector<Node*> newLeaf;
    for (int i=0, n=leaf.size(); i<n; ++i)
    {
      Node* l = leaf[i];
      Node* include = new Node;
      include->activity = activity;
      include->select = true;
      include->nF = curr_shot.size();
      include->children[0] = 0;
      include->children[1] = 0;
      setParent(include, l);
      computeMeanDistance(dists, include);
      computeInfo(include);
      newLeaf.push_back(include);

      Node* exclude = new Node;
      exclude->activity = activity;
      exclude->select = false;
      exclude->nF = curr_shot.size();
      exclude->children[0] = 0;
      exclude->children[1] = 0;
      setParent(exclude, l);
      computeMeanDistance(dists, exclude);
      computeInfo(exclude);
      newLeaf.push_back(exclude);
    }

    // Summary and Pruning
    Node min, max;
    findMinMax(newLeaf, max, min);
    NodeCompare compare(max, min);
    sort(newLeaf.begin(), newLeaf.end(), compare);

    vector<Node*> selectLeaf;
    if (shots.size() == D)
    {
      Node* n = newLeaf[0];
      Node* newRoot = n;
      while (newRoot->parent != root)
      {
        newRoot = newRoot->parent;
      }

      for (int i=0, n=newLeaf.size(), k=0; i<n && k<PRUNE_SIZE; ++i)
      {
        Node* it = newLeaf[i];
        while (it->parent != root) it = it->parent;
        if (it != newRoot) continue;

        k++;
        selectLeaf.push_back(newLeaf[i]);
      }
      root = newRoot;
      if (newRoot->select)
      {
        for (int j=0, k=shots[0].size(); j<k; ++j)
        {
          writer.write(shots[0][j]);
          printf("%d\n", 1);
        }
      }
      else
      {
        for (int j=0, k=shots[0].size(); j<k; ++j)
        {
          printf("%d\n", 0);
        }
      }
      updateRoot(newRoot, distances);
      features.pop_front();
      distances.pop_front();
      for (int i=0, n=distances.size(); i<n; ++i)
      {
        distances[i].pop_front();
      }
      shots.pop_front();
    } else {
      for (int i=0, n=newLeaf.size(), k=0; i<n && k<PRUNE_SIZE; ++i)
      {
        k++;
        selectLeaf.push_back(newLeaf[i]);
      }
    }
    leaf = selectLeaf;
  }

  // Output the remaining part
  Node* bestLeaf;
  double bestScore = 0;
  Node max, min;
  findMinMax(leaf, max, min);
  for (int i=0, n=leaf.size(); i<n; ++i)
  {
    double s = computeScore(*leaf[i], max, min);
    if (s > bestScore) {
      bestLeaf = leaf[i];
      bestScore = s;
    }
  }
  deque<Node*> stack;
  Node* it = bestLeaf;
  while (it != root)
  {
    stack.push_back(it);
    it = it->parent;
  }
  while (!stack.empty())
  {
    if (stack.back()->select)
    {
      for (int j=0, k=shots[0].size(); j<k; ++j)
      {
        writer.write(shots[0][j]);
        printf("%d\n", 1);
      }
    }
    else
    {
      for (int j=0, k=shots[0].size(); j<k; ++j)
      {
        printf("%d\n", 0);
      }
    }
    shots.pop_front();
    features.pop_front();
    stack.pop_back();
  }
}

bool readShot(VideoCapture& cap, Shot& shot)
{
  shot.clear();
  int index = 0;
  Mat frame;
  while (index < SHOT_LENGTH && cap.read(frame))
  {
    shot.push_back(frame.clone());
    index++;
  }
  return index == SHOT_LENGTH;
}
