#include "database/Dataset.h"
#include "common/Keyframe.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>
#include <list>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
using namespace std;

#define LAMDA 0.4f

typedef vector<float> Feature;

float diff(Feature& f1, Feature& f2) {
    float d = 0;
    for (int i=0; i<256; ++i) {
        float dd = f1[i] - f2[i];
        d += dd * dd;
    }
    return sqrt(d);
}

int main(int argc, char *argv[]) {
    if (argc != 4 && argc != 5) {
        printf("usage: %s <dataset_dir> <k> <output.txt> [dist_table]\n", argv[0]);
        return -1;
    }
    Dataset set(argv[1]);
    typedef pair<Feature, Keyframe> FeatureEntry;
    vector<FeatureEntry> table;

    printf("Parse feature files...\n");
    for (int i=0, n=set.getVideoInfo().size(); i<n; ++i) {
        char buf[128];
        sprintf(buf, "%s/ms_feature/%d.txt", argv[1], i);
        printf("%s\n", buf);
        FILE* ifile = fopen(buf, "r");
        float first_value = 0;
        int line_count = 0;
        while (fscanf(ifile, "%f", &first_value) == 1) {
            line_count++;
            Feature f(256);
            f[0] = first_value;
            float sum = first_value;
            for (int j=1; j<256; ++j) {
                float v;
                assert(fscanf(ifile, "%f", &v) == 1);
                f[j] = v;
                sum += fabs(v);
            }
            if (sum < 1e-6) continue;
            table.push_back(FeatureEntry(f, Keyframe(i,
                            //line_count-1 + set.getVideoInfo()[i].offset)));
                            line_count-1)));
        }
    }
    printf("Total frames: %d\n", (int) table.size());

    int n = table.size();
    float** dist_table = new float*[n];
    for (int i=0; i<n; ++i) {
        dist_table[i] = new float[n];
    }


    if (argc == 5) {
        printf("Load all pairs distance\n");
        FILE* i_table_file = fopen(argv[4], "r");
        for (int i=0; i<n; ++i) {
            read(fileno(i_table_file), dist_table[i], sizeof(float) * n);
        }
        fclose(i_table_file);
    } else {
        printf("Compute all pairs distance\n");
        for (int i=0; i<n; ++i) {
            printf("%d\n", i);
            dist_table[i][i] = 0;
            for (int j=i+1; j<n; ++j) {
                dist_table[i][j]
                        = dist_table[j][i]
                        = diff(table[i].first, table[j].first);
            }
        }

        FILE* o_table_file = fopen("dist_table.txt", "wb");
        for (int i=0; i<n; ++i) {
            write(fileno(o_table_file), dist_table[i], n * sizeof(float));
        }
        fclose(o_table_file);
    }


    int N = atoi(argv[2]);
    vector<Keyframe> summary;
    vector<bool> used(n, false);
    printf("Compute MMR\n");
    FILE* ofile = fopen(argv[3], "w");
    for (int i=0; i<N; ++i) {
        float mmr = INT_MIN;
        int mmr_i = -1;
        for (int j=0; j<n; ++j) {
            if (used[j]) continue;
            float first = 0;
            float second = 1;
            int count = 0;
            for (int k=0; k<n; ++k) {
                if (used[k]) {
                    second = min(second, dist_table[j][k]);
                } else {
                    count++;
                    first += dist_table[j][k];
                }
            }
            if (count !=0) first /= count;
            float mr = - LAMDA * first + (1 - LAMDA) * second;
            if (mr > mmr) {
                mmr = mr;
                mmr_i = j;
            }
        }
        used[mmr_i] = true;
        Keyframe k = table[mmr_i].second;
        printf("%d %d\n", k.video_id, k.frame_id);
        fprintf(ofile, "%d %d\n", k.video_id, k.frame_id);
        summary.push_back(k);
    }
}
