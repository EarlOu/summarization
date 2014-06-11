#ifndef RPI_VIDEOWRITER_H
#define RPI_VIDEOWRITER_H

#include "rpi/rpi_hardware/RpiHardwareEncoder.h"
#include <opencv2/opencv.hpp>

class RpiVideoWriter {
public:
    static RpiVideoWriter& getInstance() {
        static RpiVideoWriter instance;
        return instance;
    }

    void init(int w, int h, int fps, int fd) {
        _w = w;
        _h = h;
        _fd = fd;
        _encoder.init(w, h, fps, fd);
        _buf = new uint8_t[w * h * 4];
        _init = true;
    }

    void release() {
        if (!_init) return;
        delete[] _buf;
        _encoder.release();
        _init = false;
    }

    void write(cv::InputArray frm);

    ~RpiVideoWriter() {
        release();
    }

    int getCurrFd() {return _fd;}
private:
    RpiHardwareEncoder& _encoder;
    int _w;
    int _h;
    uint8_t* _buf;
    bool _init;
    int _fd;

    RpiVideoWriter()
            :_encoder(RpiHardwareEncoder::getInstance()), _init(false) {}
    RpiVideoWriter(const RpiVideoWriter& writer): _encoder(writer._encoder) {}
    void operator=(const RpiVideoWriter& writer) {}
};

#endif
