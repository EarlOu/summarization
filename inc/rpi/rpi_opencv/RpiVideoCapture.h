#ifndef RPI_VIDEOCAPTURE_H
#define RPI_VIDEOCAPTURE_H

#include "rpi/rpi_hardware/RpiCamReader.h"
#include <opencv2/opencv.hpp>

class RpiVideoCapture {
public:
    static RpiVideoCapture& getInstance() {
        static RpiVideoCapture instance;
        return instance;
    }

    void init(int w, int h, int fps) {
        _w = w;
        _h = h;
        _buf = new uint8_t[w * h * 4];
        _init = true;
        _reader.init(w, h, fps);
    }

    void release() {
        if (!_init) return;
        _init = false;
        _reader.release();
        delete[] _buf;
    }

    bool read(cv::OutputArray);

    ~RpiVideoCapture() {
        release();
    }

private:
    RpiCamReader& _reader;
    int _w;
    int _h;
    uint8_t* _buf;
    bool _init;

    RpiVideoCapture():_reader(RpiCamReader::getInstance()), _init(false) {}
    RpiVideoCapture(const RpiVideoCapture& cap):_reader(cap._reader) {}
    void operator=(const RpiVideoCapture& cap) {}
};

#endif
