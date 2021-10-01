#include "myftp.h"

int main(int argc, char **argv){
    char mesg[MAXLINE];
    int sockfd, n, len;
    struct sockaddr_in cliaddr, servaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(4000);

    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    for(;;){
        len = sizeof(cliaddr);
        n = recvfrom(sockfd, mesg, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
        //debug code
        puts("message received");
        //
        sendto(sockfd, mesg, n, 0, (struct sockaddr *)&cliaddr, len);
    }
}