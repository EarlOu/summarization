#include "algorithm/FeatureExtractorLch3D.h"
#include <math.h>

void FeatureExtractorLch3D::extract(InputArray iFrame, OutputArray oFeature) {
    _h_bin = (int) powl(2, _h_bit);
    _s_bin = (int) powl(2, _s_bit);
    _v_bin = (int) powl(2, _v_bit);
    _dim = _h_bin * _s_bin * _v_bin;

    Mat rgbImg = iFrame.getMat();
    Mat hsvImg;
    cvtColor(rgbImg, hsvImg, CV_BGR2HSV);
    int w = hsvImg.size().width;
    int h = hsvImg.size().height;
    int ix = 0;
    int iy = 0;
    int sw = (int) ceil(w / (double) _num_block_x);
    int sh = (int) ceil(h / (double) _num_block_y);

    oFeature.create(_dim * _num_block_y * _num_block_x, 1, CV_32FC1);
    Mat hist = oFeature.getMat();

    int index = 0;
    while (iy < h) {
        int begin_y = iy;
        int end_y = min((iy + sh), h);
        Mat row_block = hsvImg.rowRange(begin_y, end_y);
        iy = end_y;
        ix = 0;
        while (ix < w) {
            int begin_x = ix;
            int end_x = min((ix + sw), w);
            Mat block = row_block.colRange(begin_x, end_x);
            ix = end_x;
            extractBlockHist(block, hist.rowRange(index, index + _dim));
            index += _dim;
        }
    }
}

void FeatureExtractorLch3D::extractBlockHist(InputArray iBlock, OutputArray oFeature) {
    const static float MAX_H = 180.0f;
    const static float MAX_S = 255.0f;
    const static float MAX_V = 255.0f;

    Mat block = iBlock.getMat();
    int w = block.size().width;
    int h = block.size().height;

    float h_step = MAX_H / _h_bin;
    float s_step = MAX_S / _s_bin;
    float v_step = MAX_V / _v_bin;
    int count = 0;

    oFeature.create(_dim, 1, CV_32FC1);
    Mat feature = oFeature.getMat();
    feature.setTo(0);

    for (int y=0; y<h; ++y) {
        unsigned char* ptr = block.ptr<unsigned char>(y);
        for (int x=0; x<w; ++x) {
            int xx =  3 * x;
            unsigned char h = ptr[xx];
            unsigned char s = ptr[xx + 1];
            unsigned char v = ptr[xx + 2];

            int hi = min((int) floor(h/h_step), _h_bin - 1);
            int si = min((int) floor(s/s_step), _s_bin - 1);
            int vi = min((int) floor(v/v_step), _v_bin - 1);
            int i = (vi << (_h_bit + _s_bit)) + (si << _h_bit) + hi;
            ++feature.at<float>(i);
            ++count;
        }
    }
    for (int i=0; i < _dim; ++i) {
        feature.at<float>(i) /= count;
    }
}
