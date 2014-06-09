#ifndef RPI_HARDWARE_ENCODER_H
#define RPI_HARDWARE_ENCODER_H

#include <string>
using std::string;

#include <inttypes.h>

struct _COMPONENT_T;
typedef _COMPONENT_T COMPONENT_T;

class RpiHardwareEncoder {
public:
    // Singleton pattern
    static RpiHardwareEncoder& getInstance() {
        static RpiHardwareEncoder instance;
        return instance;
    }
    void init(int w, int h, int fps, int out_fd);
    void encode(uint8_t* frame);
    void release();

    ~RpiHardwareEncoder() {
        release();
    }
    int width() {return _w;}
    int height() {return _h;}

private:
    bool _init;
    int _w;
    int _h;
    int _size;
    int _fps;
    int _out_fd;
    COMPONENT_T *_video_encode_context;
    void* _client;

    RpiHardwareEncoder();
    RpiHardwareEncoder(const RpiHardwareEncoder&);
    void operator=(const RpiHardwareEncoder&);
};

#endif
