all: myftpserver myftpclient
.PHONY: all

myftpclient: myftpclient.o func.o
	gcc -o myftpclient myftpclient.o func.o

myftpserver: myftpserver.o func.o
	gcc -o myftpserver myftpserver.o func.o

myftpclient.o: myftp.h myftpclient.c
	gcc -c myftpclient.c

myftpserver.o: myftp.h myftpserver.c
	gcc -c myftpserver.c

func.o: myftp.h func.c
	gcc -c func.c