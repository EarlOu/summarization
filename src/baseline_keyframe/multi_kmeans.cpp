#include "database/Dataset.h"

#include <opencv2/opencv.hpp>
using namespace cv;

#include <string>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <vector>
using namespace std;

#include <iostream>

#define FEATURE_DIM 256

void parseFeature(const char* path, int k, vector<Mat> & features, vector<vector<int> >& index);

int main(int argc, char *argv[]) {
    printf("Multi-view keyframe extraction by performing k-means on all views together\n");
    if (argc != 5) {
        printf("usage: %s <dataset_dir> <feature_set_dir> <n> <output.txt>\n", argv[0]);
        return -1;
    }

    int n = atoi(argv[3]);
    vector<Mat> features;
    vector<vector<int> > fids;
    Dataset set(argv[1]);
    parseFeature(argv[2], set.getVideoInfo().size(), features, fids);
    FILE *ofile = fopen(argv[4], "w");
    vector<int> all_fid;
    vector<int> all_vid;

    Mat all_features;
    for (int i=0; i<fids.size(); ++i) {
        for (int j=0; j<fids[i].size(); ++j) {
            all_fid.push_back(fids[i][j]);
            all_vid.push_back(i);
        }
        if (features[i].empty()) continue;
        if (all_features.empty()) {
            all_features = features[i].clone();
        } else {
            vconcat(all_features, features[i], all_features);
        }
    }

    Mat label, center;
    kmeans(all_features, n, label, TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 100, 0.01), 1,
            KMEANS_PP_CENTERS, center);

    vector<int> best_frame(n);
    vector<float> error(n);

    for (int i=0; i<n; ++i) error[i] = INT_MAX;
    for (int i=0; i<label.size().height; ++i) {
        int l = label.at<int>(i);
        Mat f = all_features.row(i);
        Mat c = center.row(l);
        double e = norm(f, c);
        if (e < error[l]) {
            error[l] = e;
            best_frame[l] = i;
        }
    }

    for (int i=0; i<best_frame.size(); ++i) {
        int fid = all_fid[best_frame[i]];
        int vid = all_vid[best_frame[i]];
        fprintf(ofile, "%d %d\n", vid, fid);
    }
    fclose(ofile);

}

void parseFeature(const char* path, int k, vector<Mat> & features, vector<vector<int> >& fid) {
    string prefix = string(path);
    for (int file_id = 0; file_id < k; ++file_id) {
        char buf[128];
        sprintf(buf, "%s/%d.txt", path, file_id);
        printf("%s\n", buf);
        FILE* file = fopen(buf, "r");
        bool finish = false;
        vector<Mat> feature;
        vector<int> idx;
        int i = 0;
        while (!finish) {
            finish = false;
            Mat row(1, FEATURE_DIM, CV_32FC1);
            for (int j=0; j<FEATURE_DIM; ++j) {
                double v;
                if (fscanf(file, "%lf", &v) != 1) {
                    finish = true;
                    break;
                }
                row.at<float>(j) = (float) v;
            }
            if (!finish) {
                if (countNonZero(row) > 0) {
                    feature.push_back(row);
                    idx.push_back(i);
                }
            }
            ++i;
        }
        fclose(file);
        Mat f(feature.size(), FEATURE_DIM, CV_32FC1);
        for (size_t j=0; j<feature.size(); ++j) {
            Mat row = f.row(j);
            feature[j].copyTo(row);
        }
        features.push_back(f);
        fid.push_back(idx);
    }
}
