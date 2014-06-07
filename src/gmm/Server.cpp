#include "gmm/Server.h"

#include <math.h>

int Server::computeTotalLength()
{
    int maxValue = 0;
    for (int i=0, n=_sensors.size(); i<n; ++i)
    {
        maxValue = max(maxValue, _sensors[i].getEndIndex());
    }
    return maxValue;
}

void Server::run()
{
    int totalLength = computeTotalLength();
    int index = 0;

    createShotArray();

    while (index < totalLength)
    {
        // printf("Frame %d/%d:\n", index + 1, totalLength);
        vector<bool> selects;
        nextSelection(selects);
        transmitFrame(selects, index);
        // for (int i=0, n=selects.size(); i<n; ++i)
        // {
        //     printf("View %d: %s\n", i, selects[i] ? "Selected" : "Ignore");
        // }
        index++;
    }
    closeShot(index);
    vector<vector<vector<Segment> > > shots;

    for (int i=0, n=_shots.size(); i<n; ++i)
    {
        for (int j=0, m=_shots[i].size(); j<m; ++j)
        {
            printf("%d %d %d\n", i, _shots[i][j].start, _shots[i][j].end);
        }
    }

    postprocess(shots);
    write(shots);

}

void Server::nextSelection(vector<bool>& selects)
{
    vector<Mat> features;
    vector<double> scores;
    int n = _sensors.size();
    for (int i=0; i<n; ++i)
    {
        VideoSensor& sensor = _sensors[i];
        Mat frame;
        Mat feature;
        double score;
        bool select = sensor.next(frame, feature, score);
        features.push_back(select ? feature : Mat());
        scores.push_back(select ? score : 0);
        selects.push_back(select);
    }

    vector<bool> intra = selects;
    for (int i=0; i<n; ++i)
    {
        if (intra[i])
        {
            vector<Mat> recvFeatures;
            vector<double> recvScores;
            for (int j=0; j<n; ++j)
            {
                if (i==j || !intra[j]) continue;
                recvFeatures.push_back(features[j]);
                recvScores.push_back(scores[j]);
            }
            VideoSensor& sensor = _sensors[i];
            selects[i] = sensor.receiveFeature(recvFeatures, recvScores);
        }
    }
}

void Server::transmitFrame(const vector<bool>& selects, int index)
{
    for (int i=0, n=selects.size(); i<n; ++i)
    {
        if (selects[i])
        {
            int size = _shots[i].size();
            if (size == 0 || _shots[i][size - 1].end != 0)
            {
                Segment s(0, index, 0);
                _shots[i].push_back(s);
            }
        }
        else
        {
            int size = _shots[i].size();
            if (size != 0 && _shots[i][size - 1].end == 0)
            {
                _shots[i][size - 1].end = index;
            }
        }
    }
}

void Server::createShotArray()
{
    for (int i=0, n=_sensors.size(); i<n; ++i)
    {
        _shots.push_back(vector<Segment>());
    }
}

void Server::closeShot(int index)
{
    for (int i=0, n=_sensors.size(); i<n; ++i)
    {
        int s = _shots[i].size();
        if (s == 0 || _shots[i][s - 1].end != 0) continue;
        _shots[i][s - 1].end = index;
    }
}

void Server::write(vector<vector<vector<Segment> > >& shots)
{
    vector<int> indexes;
    int n = shots.size();
    for (int i=0; i<n; ++i) indexes.push_back(0);

    bool done = false;
    while (!done)
    {
        done = true;
        int firstShotStart = INT_MAX;
        int firstShotIndex = 0;
        for (int i=0; i<n; ++i)
        {
            if (indexes[i] < shots[i].size())
            {
                done = false;
                if (shots[i][indexes[i]][0].start < firstShotStart)
                {
                    firstShotStart = shots[i][indexes[i]][0].start;
                    firstShotIndex = i;
                }
            }
        }
        if (!done)
        {
            vector<Segment>& ss = shots[firstShotIndex][indexes[firstShotIndex]];
            for (int i=0, m=ss.size(); i<m; ++i)
            {
                Segment s = ss[i];
                // printf("Write Shot: %d %d %d\n", firstShotIndex, s.start, s.end);
                _sensors[firstShotIndex].write(_writer, s);
            }
            indexes[firstShotIndex]++;
        }
    }
}

void Server::postprocess(vector<vector<vector<Segment> > >& shots)
{
    for (int i=0, n=_shots.size(); i<n; ++i)
    {
        shots.push_back(vector<vector<Segment> >());
    }
    for (int i=0, n=_shots.size(); i<n; ++i)
    {
        vector<Segment>& ss = _shots[i];
        vector<vector<Segment> >& mergeShot = shots[i];
        for (int j=0, m=ss.size(); j<m; ++j)
        {
            if (mergeShot.size() != 0 && (ss[j].start - mergeShot.back().back().end) < 30)
            {
                mergeShot.back().push_back(ss[j]);
            }
            else
            {
                vector<Segment> currShot;
                currShot.push_back(ss[j]);
                mergeShot.push_back(currShot);
            }
        }

        for (int j=mergeShot.size()-1; j>=0; --j)
        {
            if (mergeShot[j].back().end - mergeShot[j].front().start < 30)
            {
                //printf("erase %d %d\n", i, j);
                mergeShot.erase(mergeShot.begin() + j);
            }
        }
    }
}
