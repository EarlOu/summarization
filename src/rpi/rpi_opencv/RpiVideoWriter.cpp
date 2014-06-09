#include "rpi_opencv/RpiVideoWriter.h"

#include <opencv2/opencv.hpp>
#include <inttypes.h>
#include <assert.h>

void RpiVideoWriter::write(cv::InputArray ifrm) {
    cv::Mat mat = ifrm.getMat();
    assert(mat.size().width == _w && mat.size().height == _h);
    uint8_t* frm_ptr = _buf;
    for (int y=0; y<_h; ++y) {
        uint8_t* ptr = mat.ptr<unsigned char>(y);
        for (int x=0; x<_w; ++x) {
           *(frm_ptr++) = ptr[2]; // R
           *(frm_ptr++) = ptr[1]; // G
           *(frm_ptr++) = ptr[0]; // B
           *(frm_ptr++) = 255; // alpha channel
           ptr+=3;
        }
    }
    _encoder.encode(_buf);
}
