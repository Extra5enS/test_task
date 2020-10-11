#ifndef CLIENT_SERVER
#define CLIENT_SERVER
#include<pthread.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

#include<sys/types.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/select.h>

#include <sys/stat.h>
#define _GNU_SOURCE
#include <fcntl.h>

#define THREAD_COUNT 64 
#define MESSAGE_SIZE (1024*8)
#define HEAD_SIZE 20
#define SEND_COUNT 5000//000
#define SERVER_PORT 8820

#define skip_head(prt) (prt + HEAD_SIZE)
#define ALL_SIZE (MESSAGE_SIZE + HEAD_SIZE)

#endif
