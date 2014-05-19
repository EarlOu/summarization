#include "FeatureExtractorEdge.h"
#include <math.h>

FeatureExtractorEdge::FeatureExtractorEdge():_edge_th(10)
{
    Mat kernel0(Size(2, 2), CV_32FC1);
    Mat kernel1(Size(2, 2), CV_32FC1);
    Mat kernel2(Size(2, 2), CV_32FC1);
    Mat kernel3(Size(2, 2), CV_32FC1);
    Mat kernel4(Size(2, 2), CV_32FC1);

    kernel0.at<float>(0,0) = 1;
    kernel0.at<float>(0,1) = 1;
    kernel0.at<float>(1,0) = -1;
    kernel0.at<float>(1,1) = -1;

    kernel1.at<float>(0,0) = 1;
    kernel1.at<float>(0,1) = -1;
    kernel1.at<float>(1,0) = 1;
    kernel1.at<float>(1,1) = -1;

    kernel2.at<float>(0,0) = sqrt(2);
    kernel2.at<float>(0,1) = 0;
    kernel2.at<float>(1,0) = 0;
    kernel2.at<float>(1,1) = -sqrt(2);

    kernel3.at<float>(0,0) = 0;
    kernel3.at<float>(0,1) = sqrt(2);
    kernel3.at<float>(1,0) = -sqrt(2);
    kernel3.at<float>(1,1) = 0;

    kernel4.at<float>(0,0) = 2;
    kernel4.at<float>(0,1) = -2;
    kernel4.at<float>(1,0) = -2;
    kernel4.at<float>(1,1) = 2;

    _kernel.push_back(kernel0);
    _kernel.push_back(kernel1);
    _kernel.push_back(kernel2);
    _kernel.push_back(kernel3);
    _kernel.push_back(kernel4);
}

void FeatureExtractorEdge::extract(InputArray iFrame, OutputArray oFeature)
{
    Mat frame = iFrame.getMat();
    Mat grayFrame;
    cvtColor(frame, grayFrame, CV_BGR2GRAY);

    Mat maxIndex(frame.size(), CV_8UC1);
    Mat maxValue = Mat::zeros(frame.size(), CV_32FC1);
    for (int i=0, n=_kernel.size(); i<n; ++i)
    {
        Mat kernel = _kernel[i];
        Mat out;
        filter2D(grayFrame, out, CV_32F, kernel, Point(0, 0), 0, BORDER_DEFAULT);
        Mat mask = out > maxValue;
        maxIndex.setTo(i, mask);
        out.copyTo(maxValue, mask);

    }
    maxIndex.setTo(255, maxValue < _edge_th);

    oFeature.create(Size(1, _kernel.size()), CV_32FC1);
    Mat feature = oFeature.getMat();
    int size = frame.size().width * frame.size().height;
    for (int i=0, n=_kernel.size(); i<n; ++i)
    {
        feature.at<float>(i) = countNonZero(maxIndex == i) / (float) size;
    }
}