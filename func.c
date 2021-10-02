#include "myftp.h"

char **prase_aguments(char *str){
    char **args;
    int cnt = 0;
    args = malloc(80);
    for (int i = 0; i < 10; i++)
        args[i] = malloc(20);
    char *p = str, *q = str;

    while (p = strchr(p, ' ')){
        *p = 0;
        //puts(q);
        //printf("%d", (int)strlen(q));
        strcpy(args[cnt++], q);
        p++;
        while (*p == ' ')
            p++;
        q = p;
    }
    // if (p = strchr(p, '\n'))
    //     *p = 0;
    if(*q){
        strcpy(args[cnt++], q);
        //puts(q);
    }
    // printf("%d", (int)strlen(q));

    return args;
} 