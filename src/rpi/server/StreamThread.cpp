#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
using std::max;

void StreamThread::run() {
    int max_fd = max(max(_fd_msg, _fd_feature), _fd_video);
    while (!_stop) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(_fd_msg, &readfds);
        FD_SET(_fd_feature, &readfds);
        FD_SET(_fd_video, &readfds);

        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("Failed in select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(_fd_msg, &readfds)) {
        }

        if (FD_ISSET(_fd_feature, &readfds)) {
        }

        if (FD_ISSET(_fd_video))
    }
}
