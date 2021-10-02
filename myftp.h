#ifndef MYFTP_H_
#define MYFTP_H_

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAXLINE 64
#define PORT     4096    // 端口号
#define LISTENQ 64
#define STATE_OFFLINE 0 //client state IDLE: wait to connect
#define STATE_OPEN_CONNECTION 1 //client try to open a connection
#define STATE_CONNECTED 2 //client connected, wait to be authentication
#define STATE_MAIN 3 //client authentication granted, now you can excute main functions 



struct message_s{
    char protocol[6];
    char type;
    char status;
    int length;
} __attribute__ ((packed));



void str_cli(FILE *fp, int sockfd);
void str_echo(int sockfd);
char **prase_aguments(char *str);


#endif