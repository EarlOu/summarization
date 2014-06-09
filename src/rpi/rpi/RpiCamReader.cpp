#include "rpi/RpiCamReader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#include <string>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

#define N_BUF 4

static int xioctl(int fh, int request, void *arg) {
    int r;

    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

static void errno_exit(const char *s) {
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

void RpiCamReader::init(int w, int h, int pixel_format) {
    if (_init) {
        fprintf(stderr, "call init() twice without calling release()\n");
        exit(EXIT_FAILURE);
    }
    if (pixel_format != CAP_BGRA8888) {
        fprintf(stderr, "Only CAP_BGRA8888 supported by this code\n");
        exit(EXIT_FAILURE);
    }
    char buf[1024];
    sprintf(buf, "uv4l --driver raspicam --encoding bgra --width %d --height %d", w, h);

    system("sudo pkill uv4l");

    if (system(buf)) {
        fprintf(stderr, "failed to init uv4l\n");
        exit(EXIT_FAILURE);
    }
    _init = true;
    open_device("/dev/uv4l");
}

void RpiCamReader::open_device(const string& dev_name) {
    struct stat st;
    if (-1 == stat(dev_name.c_str(), &st)) {
        fprintf(stderr, "Cannot identify '%s': %d, %s\n",
                dev_name.c_str(), errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (!S_ISCHR(st.st_mode)) {
        fprintf(stderr, "%s is no device\n", dev_name.c_str());
        exit(EXIT_FAILURE);
    }

    _fd = open(dev_name.c_str(), O_RDWR /* required */ , 0);

    if (-1 == _fd) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n",
                dev_name.c_str(), errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;

    if (-1 == xioctl(_fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s is no V4L2 device\n",
                    dev_name.c_str());
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "%s is no video capture device\n",
                dev_name.c_str());
        exit(EXIT_FAILURE);
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf(stderr, "%s does not support streaming i/o\n",
                dev_name.c_str());
        exit(EXIT_FAILURE);
    }

    CLEAR(cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(_fd, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl(_fd, VIDIOC_S_CROP, &crop)) {
            switch (errno) {
                case EINVAL:
                    /* Cropping not supported. */
                    break;
                default:
                    /* Errors ignored. */
                    break;
            }
        }
    } else {
        /* Errors ignored. */
    }

    /* Preserve original settings as set by v4l2-ctl for example */
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(_fd, VIDIOC_G_FMT, &fmt)) errno_exit("VIDIOC_G_FMT");

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min) fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min) fmt.fmt.pix.sizeimage = min;

    _w = fmt.fmt.pix.width;
    _h = fmt.fmt.pix.height;
    _size = _w * _h * 4;

    struct v4l2_requestbuffers req;
    enum v4l2_buf_type type;

    CLEAR(req);

    req.count = N_BUF;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(_fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s does not support "
                    "memory mapping\n", dev_name.c_str());
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) {
        fprintf(stderr, "Insufficient buffer memory on %s\n",
                dev_name.c_str());
        exit(EXIT_FAILURE);
    }

    _buffers = (buffer*) calloc(req.count, sizeof(*_buffers));

    if (!_buffers) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (_n_buffers = 0; _n_buffers < req.count; ++_n_buffers) {
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = _n_buffers;

        if (-1 == xioctl(_fd, VIDIOC_QUERYBUF, &buf))
            errno_exit("VIDIOC_QUERYBUF");

        _buffers[_n_buffers].length = buf.length;
        _buffers[_n_buffers].start =
            mmap(NULL /* start anywhere */,
                    buf.length,
                    PROT_READ | PROT_WRITE /* required */,
                    MAP_SHARED /* recommended */,
                    _fd, buf.m.offset);

        if (MAP_FAILED == _buffers[_n_buffers].start)
            errno_exit("mmap");
    }

    for (size_t i = 0; i < _n_buffers; ++i) {
        struct v4l2_buffer buf;

        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == xioctl(_fd, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF");
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(_fd, VIDIOC_STREAMON, &type))
        errno_exit("VIDIOC_STREAMON");
}

bool RpiCamReader::read(uint8_t* frame) {
    if (!_init) {
        fprintf(stderr, "call read() before calling init()\n");
        exit(EXIT_FAILURE);
    }
    struct v4l2_buffer buf;
    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    while (-1 == xioctl(_fd, VIDIOC_DQBUF, &buf)) {
        switch (errno) {
            case EAGAIN:
                continue;
            case EIO:
                /* Could ignore EIO, see spec. */
                /* fall through */
            default:
                errno_exit("VIDIOC_DQBUF");
        }
    }

    assert(buf.index < _n_buffers);
    assert(buf.bytesused == _size);

    memcpy(frame, (void*) _buffers[buf.index].start, buf.bytesused);

    if (-1 == xioctl(_fd, VIDIOC_QBUF, &buf))
        errno_exit("VIDIOC_QBUF");
    return true;
}

void RpiCamReader::close_device() {
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(_fd, VIDIOC_STREAMOFF, &type))
        errno_exit("VIDIOC_STREAMOFF");

    for (unsigned int i = 0; i < _n_buffers; ++i)
        if (-1 == munmap(_buffers[i].start, _buffers[i].length))
            errno_exit("munmap");
    free(_buffers);

    if (-1 == close(_fd))
        errno_exit("close");

    _fd = -1;
}

void RpiCamReader::release() {
    if (!_init) return;
    close_device();
    system("sudo pkill uv4l");
    _init = false;
}

#undef CLEAR
#undef N_BUF
