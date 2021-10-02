#include "myftp.h"

const char ftp_protocol[6] = {0xe3, 'm', 'y', 'f', 't', 'p'};

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

        int client_state = STATE_OPEN_CONNECTION; 
        //start to serve for the current client connected
again:
        while((n = read(connfd, &msg_buf, sizeof(msg_buf))) > 0){
            
            //debug code
            //printf("msg_type=%x\n", (char)msg_buf.type);

            //get an OPEN_CONN_REQUEST
            if (msg_buf.type == (char)0xA1){
                
                //send an OPEN_CONN_REPLY
                struct message_s open_conn_reply;
                memcpy(open_conn_reply.protocol, ftp_protocol, 6);
                open_conn_reply.type = 0xA2;
                open_conn_reply.status = 1;
                open_conn_reply.length = 12;
                write(connfd, &open_conn_reply, sizeof(open_conn_reply));
            }
            //get an AUTH_REQUEST
            else if (msg_buf.type == (char)0xA3){
                char *payload_buf;
                payload_buf = malloc(100);
                struct message_s auth_reply;
                memcpy(auth_reply.protocol, ftp_protocol, 6);
                auth_reply.type = 0xA4;
                auth_reply.status = 0;
                auth_reply.length = 12;

                //get the username and passwd from client
                if ((n = read(connfd, payload_buf, msg_buf.length - 12)) < 0){
                    fprintf(stderr, "fail to read the authentication payload! permission denied!\n");
                    //send a AUTH_REPLY with status=0
                    write(connfd, &auth_reply, sizeof(auth_reply));
                    continue;
                }

                //check username and passwd in access.txt
                FILE *fp;
                char str_buf[MAXLINE];
 
                if((fp=fopen("access.txt", "rt")) == NULL){
                    fprintf(stderr, "Cannot open access.txt！\n");
                    write(connfd, &auth_reply, sizeof(auth_reply));
                    continue;
                }

                int matched = 0;
                while(fgets(str_buf, MAXLINE, fp) != NULL){
                    str_buf[strlen(str_buf) - 1] = 0;
                    //passwd matched
                    if (strcmp(str_buf, payload_buf) == 0){
                        auth_reply.status = 1;
                        write(connfd, &auth_reply, sizeof(auth_reply));
                        printf("client authenticated!\n");
                        client_state = STATE_MAIN;
                        matched = 1;
                        break;
                    }
                }

                //can't find a match, permission denied
                if (!matched){
                    write(connfd, &auth_reply, sizeof(auth_reply));
                    printf("username or passwd wrong!\n");
                }
 
                fclose(fp);
            }
            //get an LIST_REQUEST
            else if (msg_buf.type == (char)0xA5){
                struct message_s list_reply;
                memcpy(list_reply.protocol, ftp_protocol, 6);
                list_reply.type = 0xA6;
                list_reply.length = 12;
                list_reply.status = 0;
                

                if (client_state == STATE_MAIN){
                    //to get the file list
     	            DIR *dir = NULL;  
                    struct dirent entry;  
                    struct dirent *result;
                    char name_buf[500];
                    char *p = name_buf;
      
                    if((dir = opendir("filedir"))==NULL){  
                        printf("failed to open filedir!\n");  
                        continue;  
                    }
	                
     		        while(!readdir_r(dir, &entry, &result) && result){
                        if (entry.d_type == (char)8){
                            strcpy(p, entry.d_name);
                            p = p + strlen(p);
                            *p = '\n';
                            p++;
                        }
                    }
                    *p = 0;
     		        closedir(dir); 

                    //send the list reply
                    list_reply.status = 1;
                    list_reply.length += strlen(name_buf) + 1;
                    write(connfd, &list_reply, sizeof(list_reply));
                    write(connfd, name_buf, strlen(name_buf) + 1);
                    printf("file list sent\n");
    
                }
                else{
                    printf("client try to list files without authentication!\n");
                    write(connfd, &list_reply, sizeof(list_reply));
                }
            }
        }
        if (n < 0 && errno == EINTR)
            goto again; //soft interrupt, try again
        else if (n < 0){
            perror("fail to get the protocol message!\n");
            continue;
        }
        
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