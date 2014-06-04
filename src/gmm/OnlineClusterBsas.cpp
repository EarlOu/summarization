#include "gmm/OnlineClusterBsas.h"
#include <limits.h>
#include <math.h>

int OnlineClusterBsas::cluster(InputArray iFeature, OutputArray oResidue) {
    Mat feature = iFeature.getMat();
    oResidue.create(feature.size(), feature.type());
    Mat residue = oResidue.getMat();
    double minDiff = INT_MAX;
    int index = 0;

    bool isBackground = false;
    for (int i=0, n=_centroid.size(); i<n; ++i) {
        double diff = norm(feature, _centroid[i]);
        if (diff < minDiff) {
            index = i;
            minDiff = diff;
        }
        if (isBackground || (diff < _diffTh && _histCount[i] > _backgroundHistTh)) isBackground = true;
    }

    if (minDiff < _diffTh) {
        Mat orig = _centroid[index];
        double rate = max(_learningRate, 1.0/(_histCount[index]+1.0));
        _centroid[index] = orig * (1 - rate) + feature * rate;
        _histCount[index]++;
        _lastCount[index] = -1;
    } else {
        if ((int)_centroid.size() < _maxNumCluster) {
            index = _centroid.size();
            _centroid.push_back(feature);
            _histCount.push_back(1);
            _lastCount.push_back(-1);
        } else {
            int minHist = INT_MAX;
            int minHistIndex = 0;
            for (int i=0, n=_histCount.size(); i<n; ++i) {
                if (_histCount[i] < minHist) {
                    minHist = _histCount[i];
                    minHistIndex = i;
                }
            }
            if (minHist < _histReplaceTh) {
                _centroid[minHistIndex] = feature;
                _histCount[minHistIndex] = 1;
                _lastCount[minHistIndex] = -1;
            }
            index = minHistIndex;
        }
    }

    for (int i=0, n=_lastCount.size(); i<n; ++i) {
        _lastCount[i]++;
        if (_lastCount[i] > _lastSeenTh) {
            _histCount[i]--;
            if (_histCount[i] < 0) _histCount[i] = 0;
        }
    }

    // Cluster merge
    for (int i = _centroid.size() - 1; i >=0; --i) {
        double d = INT_MAX;
        int idx_j = 0;
        for (int j=0; j<i; ++j) {
            double dd = norm(_centroid[i], _centroid[j]);
            if (dd < d) {
                d = dd;
                idx_j = j;
            }
        }
        if (d < _mergeTh) {
            _centroid[idx_j] = (_centroid[idx_j] * _histCount[idx_j] + _centroid[i] * _histCount[i]) / (_histCount[idx_j] + _histCount[i]);
            _histCount[idx_j] = (_histCount[idx_j] + _histCount[i]);
            _lastCount[idx_j] = min(_lastCount[idx_j], _lastCount[i]);
            _centroid.erase(_centroid.begin() + i);
            _histCount.erase(_histCount.begin() + i);
            _lastCount.erase(_lastCount.begin() + i);
            if (index == i) index = idx_j;
        }
    }

    return index;
}

double OnlineClusterBsas::diff(InputArray iFeature, int cluster) {
    return norm(_centroid[cluster], iFeature);
}

bool OnlineClusterBsas::isMatched(InputArray iFeature, int cluster) {
    return ( diff(iFeature, cluster) < _diffTh);
}

int OnlineClusterBsas::getHistCount(int c) {
    return _histCount[c];
}

bool OnlineClusterBsas::isBackground(int cluster) {
    return _histCount[cluster] > _backgroundHistTh;
}

void OnlineClusterBsas::getCenter(int cluster, OutputArray oCenter) {
    Mat center = _centroid[cluster];
    oCenter.create(center.size(), center.type());
    center.copyTo(oCenter.getMat());
}
