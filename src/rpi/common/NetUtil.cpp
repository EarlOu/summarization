#include "rpi/common/NetUtil.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int connect_to(int32_t s_addr, int port) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to open socket");
        exit(EXIT_FAILURE);
    }

    sockaddr_in serveraddr;
    memset((void*) &serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = s_addr;

    if (connect(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) < 0) {
        perror("Failed to connect");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int listen_connect(int port, int max_connect) {
    sockaddr_in my_addr;
    int sockoptval = 1;

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to open listen socket");
        exit(EXIT_FAILURE);
    }

    // set port resuable
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));

    memset((void*) &my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr*) &my_addr, sizeof(my_addr)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, max_connect) < 0) {
        perror("Failed to listen");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

int accept_connect(int sockfd) {
    sockaddr_in client_addr;
    socklen_t alen = sizeof(client_addr);
    int rqst;
    if ((rqst = accept(sockfd, (struct sockaddr*) &client_addr, &alen)) < 0) {
        perror("Failed to accept");
        exit(EXIT_FAILURE);
    }

    close(sockfd);
    return rqst;
}

int sendall(int sockfd, char* buf, int len) {
    int total = 0;

    while (total < len) {
        int n = send(sockfd, buf + total, len - total, 0);
        if (n < 0) return n;
        total += n;
    }

    return total;
}

int recvall(int sockfd, char* buf, int len) {
    int total = 0;

    while (total < len) {
        int n = recv(sockfd, buf + total, len - total, 0);
        if (n <= 0) return n;
        total += n;
    }

    return total;
}
