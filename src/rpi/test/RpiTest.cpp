#include "rpi/RpiCamReader.h"
#include "rpi/RpiHardwareEncoder.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    RpiCamReader& cap = RpiCamReader::getInstance();
    cap.init(320, 240);

    RpiHardwareEncoder& writer = RpiHardwareEncoder::getInstance();
    int o_fd = open("out.h264", O_WRONLY | O_CREAT,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (o_fd < 0) {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }
    writer.init(cap.width(), cap.height(), 30, o_fd);

    int i = 0;
    uint8_t* frame = new uint8_t[cap.size()];
    while(i++ < 30) {
        cap.read(frame);
        writer.encode(frame);
    }
    close(o_fd);
}
