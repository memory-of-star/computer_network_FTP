#include "myftp.h"



int main(int argc, char **argv){
    int listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    struct message_s msg_buf;
    int n;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);

    for(;;){
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        //debug code
        printf("get a connection\n");
        //
        //str_echo(connfd);

        //get an OPEN_CONN_REQUEST
        if (read(connfd, &msg_buf, sizeof(msg_buf)) < 0){
            fprintf(stderr, "fail to get OPEN_CONN_REQUEST\n");
            continue;
        }

        //send an OPEN_CONN_REPLY
        struct message_s open_conn_reply;
        memcpy(open_conn_reply.protocol, ftp_protocol, 6);
        open_conn_reply.type = 0xA2;
        open_conn_reply.status = 1;
        open_conn_reply.length = 12;
        write(connfd, &open_conn_reply, sizeof(open_conn_reply));
    }
}

void str_echo(int sockfd){
    ssize_t n;
    char buf[MAXLINE];

again:
    while((n = read(sockfd, buf, MAXLINE)) > 0)
        write(sockfd, buf, n);
    if (n < 0 && errno == EINTR)
        goto again; //soft interrupt, try again
    else if (n < 0){
        perror("fail to read");
        exit(1);
    }
    //debug code
    //printf("%d\n", (int)n);
    //
}