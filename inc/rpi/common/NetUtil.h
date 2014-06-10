#ifndef NET_UTIL_H
#define NET_UTIL_H

#include <sys/types.h>

int connect_to(int32_t s_addr, int port);
int listen_connect(int port, int max_connect = 1);
int accept_connect(int sockfd);
int sendall(int sockfd, char* buf, int len);
int recvall(int sockfd, char* buf, int len);

#endif
