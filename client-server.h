#ifndef CLIENT_SERVER
#define CLIENT_SERVER

#include<pthread.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>

#include<sys/types.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#include <sys/stat.h>
#include <fcntl.h>

#define THREAD_COUNT 64 // N 
#define MESSAGE_SIZE 1024*8

#endif
