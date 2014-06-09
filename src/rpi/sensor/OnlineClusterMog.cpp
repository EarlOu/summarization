#include "sensor/OnlineClusterMog.h"
#include <math.h>
#include <limits.h>
#include <algorithm>
#include <opencv2/opencv.hpp>
using namespace cv;

#define PI 3.141593

int OnlineClusterMog::cluster(InputArray iFeature, OutputArray oResidue)
{
    Mat feature = iFeature.getMat();

    if (_cluster.size() > 0)
    {
        CV_Assert(feature.rows == _cluster[0].mean.rows &&
                feature.cols == 1 &&
                feature.type() == CV_32FC1);
    }

    // Update matched cluster
    bool matched = false;
    double total_weight = 0;
    int matchIdx = 0;
    int maxLikelihood = -INT_MAX;

    for (int i=0, n=_cluster.size(); i<n; ++i)
    {
        Mat diff = (feature - _cluster[i].mean);
        double accuDiff = sum(diff.mul(diff))[0];
        double b = accuDiff / _cluster[i].sigma;
        double likelihood = - 0.5 * diff.rows * log(_cluster[i].sigma) - 0.5 * b;

        // printf("%d %lf %lf %lf %lf\n", i, accuDiff, b, _cluster[i].sigma, _cluster[i].weight);
        if (b < _K && likelihood > maxLikelihood)
        {
            maxLikelihood = likelihood;
            matchIdx = i;
            matched = true;
        }
    }

    for (int i=0, n=_cluster.size(); i<n; ++i)
    {
        if (i == matchIdx)
        {
            Mat diff = (feature - _cluster[i].mean);
            double accuDiff = sum(diff.mul(diff))[0];
            double b = accuDiff / _cluster[i].sigma;
            double r = exp(-0.5 * b);
            _cluster[i].weight = (1 - _alpha) * _cluster[i].weight + _alpha;
            _cluster[i].mean = (1 - _alpha * r) * _cluster[i].mean + _alpha * r * feature;
            _cluster[i].sigma = (1 - _alpha * r) * _cluster[i].sigma + _alpha * r * accuDiff;
        }
        else
        {
            _cluster[i].weight = (1 - _alpha) * _cluster[i].weight;
        }
        total_weight += _cluster[i].weight;
    }

    // Re-normalize weight
    for (int i=0, n=_cluster.size(); i<n; ++i)
    {
        _cluster[i].weight /= total_weight;
    }

    // Create new cluster if no matched
    int idx = matchIdx;
    if (!matched)
    {
        idx = _cluster.size();
        if (idx >= _K)
        {
            double minWeight = INT_MAX;
            for (int i=0, n=_cluster.size(); i<n; ++i)
            {
                double w = _cluster[i].weight;
                if (w < minWeight)
                {
                    minWeight = w;
                    idx = i;
                }
            }
        } else {
            Cluster c;
            _cluster.push_back(c);
        }

        _cluster[idx].mean = feature.clone();
        _cluster[idx].sigma = 0.005;
        _cluster[idx].weight = 0.01 / _K;

        // Re-normalized again
        total_weight = 0;
        for (int i=0, n=_cluster.size(); i<n; ++i)
        {
            total_weight += _cluster[i].weight;
        }
        for (int i=0, n=_cluster.size(); i<n; ++i)
        {
            _cluster[i].weight /= total_weight;
        }
    }

    return idx;
}

bool OnlineClusterMog::isBackground(int cluster)
{
    vector<Cluster> c = _cluster;
    for (int i=0, n=c.size(); i<n; ++i)
    {
        c[i].index = i;
    }

    std::sort(c.begin(), c.end(), ClusterComparator());
    double w = 0;
    for (int i=0, n=c.size(); i<n; ++i)
    {
        if (cluster == c[i].index) return true;
        w += c[i].weight;
        if (w > _T)
        {
            if (i > 0 && fabs(c[i].weight - c[0].weight) < 0.01) return true;
            return false;
        }
    }
    return false;
}

void OnlineClusterMog::getBackground(OutputArray back)
{
    vector<Cluster> c = _cluster;
    for (int i=0, n=c.size(); i<n; ++i)
    {
        c[i].index = i;
    }

    std::sort(c.begin(), c.end(), ClusterComparator());
    back.create(c[0].mean.size(), c[0].mean.type());
    Mat b = back.getMat();
    c[0].mean.copyTo(b);
}

int OnlineClusterMog::getTrueBackground(InputArray iFeature)
{
    Mat feature = iFeature.getMat();
    vector<Cluster> c = _cluster;
    for (int i=0, n=c.size(); i<n; ++i)
    {
        c[i].index = i;
    }

    std::sort(c.begin(), c.end(), ClusterComparator());
    double w = 0;
    double max_likelihood = -INT_MAX;
    int idx = 0;

    for (int i=0, n=c.size(); i<n; ++i)
    {
        Mat diff = (feature - c[i].mean);
        double accuDiff = sum(diff.mul(diff))[0];
        double b = accuDiff / c[i].sigma;
        double likelihood = log(c[i].weight) - 0.5 * log(diff.rows * _cluster[i].sigma) - 0.5 * b;
        if (likelihood > max_likelihood)
        {
            max_likelihood = likelihood;
            idx = c[i].index;
        }


        if (w > _T)
        {
            if (!(i > 0 && fabs(c[i].weight - c[i-1].weight) < 0.01))
            {
                return idx;
            }
        }
        w += c[i].weight;
    }
    return idx;
}

void OnlineClusterMog::getCenter(int cluster, OutputArray oCenter)
{
    Mat center = _cluster[cluster].mean;
    oCenter.create(center.size(), center.type());
    Mat mat = oCenter.getMat();
    center.copyTo(mat);
}
