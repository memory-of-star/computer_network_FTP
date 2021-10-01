#include "myftp.h"

int main(int argc, char **argv)
{
    int sockfd, n;
    struct sockaddr_in servaddr;
    char sendline[MAXLINE], recvline[MAXLINE + 1];
    if (argc != 2){
        fprintf(stderr, "%s\n", "usage: echoCli <IPaddress>");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    servaddr.sin_port = htons(4000);

    // debug code
    // printf("debug point 1");
    // char *p = fgets(sendline, MAXLINE, stdin);
    // puts(sendline);
    // printf("%d", (int)p);
    // p = fgets(sendline, MAXLINE, stdin);
    // puts(sendline);
    // printf("%d", (int)p);
    //
    
    while (fgets(sendline, MAXLINE, stdin) != NULL){
        //debug code
        puts("send a message");
        //
        sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
        //debug code
        puts("message sent");
        //
        n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
        //debug code
        puts("message received");
        //
        recvline[n] = 0;
        //debug code
        printf("%d\n", n);
        //
        fputs(recvline, stdout);
    }
    exit(0);
}