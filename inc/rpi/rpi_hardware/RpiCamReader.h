#ifndef PRI_CAM_READER_H
#define PRI_CAM_READER_H

#include <string>
using std::string;

#include <inttypes.h>

const int CAP_BGRA8888 = 0;

class RpiCamReader {
public:
    // Singleton pattern
    static RpiCamReader& getInstance() {
        static RpiCamReader instance;
        return instance;
    }
    void init(int w, int h, int pixel_type = CAP_BGRA8888);
    bool read(uint8_t* frame);
    void release();

    ~RpiCamReader() {
         release();
    }
    int width() {return _w;}
    int height() {return _h;}
    int size() {return _size;}
private:
    struct buffer {
        void   *start;
        size_t  length;
    };

    bool _init;
    void open_device(const string& dev_name);
    void close_device();
    int _fd;
    int _n_buffers;
    int _w;
    int _h;
    int _size;
    struct buffer *_buffers;

    RpiCamReader(): _init(false) {}
    RpiCamReader(const RpiCamReader&);
    void operator=(const RpiCamReader&);
};

#endif
