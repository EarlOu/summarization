#include "rpi_opencv/RpiVideoCapture.h"

#include <opencv2/opencv.hpp>
#include <inttypes.h>

bool RpiVideoCapture::read(cv::OutputArray oframe) {
    oframe.create(_h, _w, CV_8UC3);
    cv::Mat mat = oframe.getMat();

    _reader.read(_buf); // read bgra frame

    uint8_t* frm_ptr = _buf;
    for (int y=0; y<_h; ++y) {
        uint8_t* ptr = mat.ptr<uint8_t>(y);
        for (int x=0; x<_w; ++x) {
            *(ptr++) = *(frm_ptr++);
            *(ptr++) = *(frm_ptr++);
            *(ptr++) = *(frm_ptr++);
            frm_ptr++;
        }
    }

    return true;
}
