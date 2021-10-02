#include "myftp.h"

const char ftp_protocol[6] = {0xe3, 'm', 'y', 'f', 't', 'p'};

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;
    char str[MAXLINE];
    char **args;
    struct message_s msg_buf;
    int state = STATE_OFFLINE;

    printf("$");
    //start the shell loop
    while(fgets(str, MAXLINE, stdin) != NULL){
        
        char *p = strchr(str, '\n');
        if (p)
            *p = 0;
        args = prase_aguments(str);
        //debug code
        // for(int i = 0; i < 10; i++)
        //     if (strlen(args[i]) > 0)
        //         puts(args[i]);
        //

        //get an open command
        if (strcmp(args[0], "open") == 0){
            if (state == STATE_OFFLINE){
                //setup a TCP connection
                sockfd = socket(AF_INET, SOCK_STREAM, 0);
                bzero(&servaddr, sizeof(servaddr));
                servaddr.sin_family = AF_INET;
                servaddr.sin_port = htons(atoi(args[2]));
                //debug code
                // printf("args[2]=%s\n", args[2]);
                // printf("args[1]=%s\n", args[1]);
                //
                inet_pton(AF_INET, args[1], &servaddr.sin_addr);
                if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
                    fprintf(stderr, "TCP connection failed\n");
                    printf("$");
                    continue;
                }

                //then send a protocol message OPEN_CONN_REQUEST to the server
                struct message_s open_conn_request;
                memcpy(open_conn_request.protocol, ftp_protocol, 6);
                open_conn_request.type = 0xA1;
                open_conn_request.length = 12;
                write(sockfd, &open_conn_request, sizeof(open_conn_request));

                //receive the server's reply
                if(read(sockfd, &msg_buf, sizeof(msg_buf)) != sizeof(msg_buf) || msg_buf.type != (char)0xA2){
                    fprintf(stderr, "fail to get OPEN_CONN_REPLY\n");
                    printf("$");
                    continue;
                }
                if (msg_buf.status == 0){
                    fprintf(stderr, "connection rejected\n");
                    printf("$");
                    continue;
                }
                state = STATE_CONNECTED;
                fprintf(stdout, "conection setup successfully\n");
            }
            else
                printf("you have already connected!\n");
        }
        //get an auth command
        else if (strcmp(args[0], "auth") == 0){
            if (state == STATE_CONNECTED){
                //send a AUTH_REQUEST to the server
                struct message_s auth_request;
                char *payload;
                payload = malloc(100); //we assume that the username + passwd + 2 < 100

                memcpy(auth_request.protocol, ftp_protocol, 6);
                auth_request.type = 0xA3;
                auth_request.length = 12 + strlen(args[1]) + strlen(args[2]) + 2;
                strcpy(payload, args[1]);
                payload[strlen(args[1])] = ' ';
                strcpy(payload + strlen(args[1]) + 1, args[2]);

                write(sockfd, &auth_request, sizeof(auth_request));
                write(sockfd, payload, strlen(payload) + 1);

                // recevie a AUTH_REPLY
                if(read(sockfd, &msg_buf, sizeof(msg_buf)) != sizeof(msg_buf) || msg_buf.type != (char)0xA4){
                    state = STATE_OFFLINE;
                    close(sockfd);
                    fprintf(stderr, "fail to get AUTH_REPLY\n");
                    printf("$");
                    continue;
                }
                if (msg_buf.status == 0){
                    state = STATE_OFFLINE;
                    close(sockfd);
                    fprintf(stderr, "authentication rejected\n");
                    printf("$");
                    continue;
                }
                state = STATE_MAIN;
                fprintf(stdout, "authentication get successfully\n");

            }
            else if (state == STATE_OFFLINE)
                printf("you need to open a connection before authentication!\n");
            else if (state == STATE_MAIN)
                printf("you have already authenticated!\n");
        }
        //get a ls command, list the files
        else if (strcmp(args[0], "ls") == 0){
            if (state == STATE_MAIN){
                //then send a protocol message LIST_REQUEST to the server
                struct message_s list_request;
                memcpy(list_request.protocol, ftp_protocol, 6);
                list_request.type = 0xA5;
                list_request.length = 12;
                write(sockfd, &list_request, sizeof(list_request));

                //get the file list reply
                if(read(sockfd, &msg_buf, sizeof(msg_buf)) != sizeof(msg_buf) || msg_buf.type != (char)0xA6){
                    fprintf(stderr, "fail to get LIST_REPLY\n");
                    printf("$");
                    continue;
                }
                if (msg_buf.status == 0){
                    fprintf(stderr, "list rejected\n");
                    printf("$");
                    continue;
                }
                //get the file name string
                char name_buf[500];
                if (read(sockfd, &name_buf, msg_buf.length - 12) != (msg_buf.length - 12)){
                    fprintf(stderr, "fail to get file list string payload!\n");
                    printf("$");
                    continue;
                }
                fprintf(stdout, "file list get successfully, and show as below:\n");
                printf("%s", name_buf);
            }
            else
                printf("you need to authenticate first!\n");
        }
        // a get command, to download a file
        else if (strcmp(args[0], "get") == 0){
            if (state == STATE_MAIN){
                //then send a protocol message GET_REQUEST to the server
                struct message_s get_request;
                memcpy(get_request.protocol, ftp_protocol, 6);
                get_request.type = 0xA7;
                get_request.length = 12 + strlen(args[1]) + 1;
                write(sockfd, &get_request, sizeof(get_request));
                write(sockfd, args[1], strlen(args[1]) + 1);

                //get the GET_REPLY
                if(read(sockfd, &msg_buf, sizeof(msg_buf)) != sizeof(msg_buf) || msg_buf.type != (char)0xA8){
                    fprintf(stderr, "fail to get GET_REPLY\n");
                    printf("$");
                    continue;
                }
                if (msg_buf.status == 0){
                    fprintf(stderr, "download rejected\n");
                    printf("$");
                    continue;
                }
                //now download is allowed, we need to get file data
                char file_buf[MAX_FILE];

                if(read(sockfd, &msg_buf, sizeof(msg_buf)) != sizeof(msg_buf) || msg_buf.type != (char)0xFF){
                    fprintf(stderr, "fail to get FILE_DATA\n");
                    printf("$");
                    continue;
                }

                if (read(sockfd, &file_buf, msg_buf.length - 12) != (msg_buf.length - 12)){
                    fprintf(stderr, "fail to get file data payload!\n");
                    printf("$");
                    continue;
                }
                
                fprintf(stdout, "file downloaded successfully\n");

                FILE *fp;
                if((fp=fopen(args[1], "wb")) == NULL){
                    fprintf(stderr, "Cannot open(or create) %s！\n", args[1]);
                    printf("$");
                    continue;
                }
                fwrite(file_buf, sizeof(char), msg_buf.length - 12, fp);
                fprintf(stdout, "file stored successfully\n");
                fclose(fp);
            }
            else
                printf("you need to authenticate first!\n");
        }
        // put command, to upload a file
        else if (strcmp(args[0], "put") == 0){
            if (state == STATE_MAIN){
                //first check whether the file is in the local
                FILE *fp;
                if ((fp = fopen(args[1], "rb")) == NULL){
                    printf("there is no file named %s\n", args[1]);
                    printf("$");
                    continue;
                }
                char file_buf[MAX_FILE];

                fseek(fp, 0, SEEK_END);
                int size = ftell(fp);
                fseek(fp, 0, SEEK_SET);

                if(fread(file_buf, size, 1, fp) == 0){
                    fprintf(stderr, "reading %s failed！\n", args[1]);
                    fclose(fp);
                    printf("$");
                    continue;
                }
                fclose(fp);
                //then send a protocol message PUT_REQUEST to the server
                struct message_s put_request;
                memcpy(put_request.protocol, ftp_protocol, 6);
                put_request.type = 0xA9;
                put_request.length = 12 + strlen(args[1]) + 1;
                write(sockfd, &put_request, sizeof(put_request));
                write(sockfd, args[1], strlen(args[1]) + 1);

                //get the PUT_REPLY
                if(read(sockfd, &msg_buf, sizeof(msg_buf)) != sizeof(msg_buf) || msg_buf.type != (char)0xAA){
                    fprintf(stderr, "fail to get PUT_REPLY\n");
                    printf("$");
                    continue;
                }
                if (msg_buf.status == 0){
                    fprintf(stderr, "upload rejected\n");
                    printf("$");
                    continue;
                }
                //now upload is allowed, we need to push file data
                struct message_s file_data;
                memcpy(file_data.protocol, ftp_protocol, 6);
                file_data.type = 0xFF;
                file_data.length = 12 + size;
                write(sockfd, &file_data, sizeof(file_data));
                write(sockfd, file_buf, size);
                printf("file sent\n");

            }
            else
                printf("you need to authenticate first!\n");
        }

        printf("$");
    }
    exit(1);
}


