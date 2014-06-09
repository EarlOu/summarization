#include "rpi/common/Common.h"
#include "rpi/common/NetUtil.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <vector>
using std::vector;

const int MAX_N_SENSOR = 10;

struct ConnectInfo {
    int fd_msg;
    int fd_feature;
    int fd_video;
};

static int listen_connection(int port) {
    sockaddr_in my_addr;
    int sockoptval = 1;

    int svc;
    if ((svc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to open listen socket");
        exit(EXIT_FAILURE);
    }
    fcntl(svc, F_SETFL, O_NONBLOCK);

    // set port resuable
    setsockopt(svc, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));

    memset((void*) &my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(svc, (struct sockaddr*) &my_addr, sizeof(my_addr)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    if (listen(svc, MAX_N_SENSOR) < 0) {
        perror("Failed to listen");
        exit(EXIT_FAILURE);
    }
    return svc;
}


void wait_connection(int svc, vector<ConnectInfo>& client) {
    int rqst;

    fd_set readfds;
    while(true) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(svc, &readfds);
        if (select(svc + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("Failed to select");
            exit(EXIT_FAILURE);
        }

        sockaddr_in client_addr;
        socklen_t alen = sizeof(client_addr);

        if (FD_ISSET(svc, &readfds)) {
            if ((rqst = accept(svc, (struct sockaddr*) &client_addr, &alen)) < 0) {
                if (rqst == EWOULDBLOCK) continue;
                perror("Failed to accept");
                exit(EXIT_FAILURE);
            }

            printf("Accept connection from: %s:%d, fd = %d\n",
                    inet_ntoa(client_addr.sin_addr), (int) ntohs(client_addr.sin_port), rqst);
            ConnectInfo c;
            c.fd_msg = rqst;
            c.fd_feature = connect_to(client_addr.sin_addr.s_addr, PORT_FEATURE);
            printf(">>> Connect to feature channel, fd = %d\n", c.fd_feature);
            c.fd_video = connect_to(client_addr.sin_addr.s_addr, PORT_VIDEO);
            printf(">>> Connect to video channel, fd = %d\n", c.fd_video);
            client.push_back(c);
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char buf[1024];
            scanf("%1023s", buf);
            if (!strcmp(buf, "start")) {
                break;
            } else {
                printf("Unknonw command: %s\n", buf);
            }
        }
    }
}


int main(int argc, char *argv[]) {
    printf("Listen to port: %d\n", PORT);
    int sock_fd = listen_connection(PORT);

    vector<ConnectInfo> client_fds;
    printf("Type 'start' to begin the system.\n");
    printf("Wait for connection...\n");
    wait_connection(sock_fd, client_fds);
}
