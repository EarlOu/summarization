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

void parseFeature(const char* path, vector<Mat> & features, vector<vector<int> >& index);

int main(int argc, char *argv[]) {
    printf("Multi-view keyframe extraction by performing k-means on each view\n");
    if (argc != 5) {
        printf("usage: %s <dataset_dir> <feature_set_dir> <n> <output.txt>\n", argv[0]);
        return -1;
    }

    int n = atoi(argv[3]);
    vector<Mat> features;
    vector<vector<int> > fids;
    Dataset set(argv[1]);
    parseFeature(argv[2], features, fids);
    vector<VideoInfo>& info = set.getVideoInfo();
    FILE *ofile = fopen(argv[4], "w");

    for (int vid=0; vid<info.size(); ++vid) {
        printf("%d\n", vid);

        Mat feature = features[vid];
        Mat label, center;
        int n_cluster = n / info.size();
        n_cluster = n_cluster < feature.size().height ? n_cluster : feature.size().height;
        if (n_cluster == 0) continue;

        kmeans(feature, n_cluster, label, TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 100, 0.01), 1,
                KMEANS_PP_CENTERS, center);
        vector<int> best_frame(n_cluster);
        vector<float> error(n_cluster);

        for (int i=0; i<n_cluster; ++i) error[i] = INT_MAX;
        for (int i=0; i<label.size().height; ++i) {
            int l = label.at<int>(i);
            Mat f = feature.row(i);
            Mat c = center.row(l);
            double e = norm(f, c);
            if (e < error[l]) {
                error[l] = e;
                best_frame[l] = fids[vid][i];
            }
        }

        for (int i=0; i<best_frame.size(); ++i) {
            int fid = best_frame[i];
            fprintf(ofile, "%d %d\n", vid, fid);
        }
    }

    fclose(ofile);

}

void parseFeature(const char* path, vector<Mat> & features, vector<vector<int> >& fid) {
    string prefix = string(path);
    prefix = prefix.substr(0, prefix.find_last_of('/'));

    char buf[1024];
    FILE* ifile = fopen(path, "r");
    while(fscanf(ifile, "%s", buf) == 1) {
        string filepath = prefix + '/' + string(buf);
        printf("%s\n", filepath.c_str());
        FILE* file = fopen(filepath.c_str(), "r");
        bool finish = false;
        vector<Mat> feature;
        vector<int> idx;
        int i = 0;
        while (!finish) {
            finish = false;
            Mat row(1, FEATURE_DIM, CV_32FC1);
            for (int i=0; i<FEATURE_DIM; ++i) {
                double v;
                if (fscanf(file, "%lf", &v) != 1) {
                    finish = true;
                    break;
                }
                row.at<float>(i) = (float) v;
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
        for (size_t i=0; i<feature.size(); ++i) {
            feature[i].copyTo(f.row(i));
        }
        features.push_back(f);
        fid.push_back(idx);
    }
    fclose(ifile);
}
