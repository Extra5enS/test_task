#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<string.h>

#include<sys/types.h>
#include<sys/socket.h>

#include <sys/stat.h>
#include <fcntl.h>


#define SEND_COUNT 5000000
#define THREAD_COUNT 64 // N 
#define MESSAGE_SIZE 1024*8

typedef struct {
    int num;
    char * text;
} message_t;

void *client_thread() {
    /* Here must be 5'000'000 messages about 8k every one
     * But here we will send one messag 5'000'000 times */
    int socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	struct sockaddr server;
    connect(socket_desc , &server , sizeof(server));
    
    for(int i = 0; i < SEND_COUNT; i++) {
        char message[MESSAGE_SIZE]; 
        // first vertion, in next we will creat list with last 10 messages
        
        int fd = open("test_message", O_RDONLY); 
        // open 5'000'000 times to simulate 5'000'000 diferent massages 
        read(fd, message, MESSAGE_SIZE);
        
        send(socket_desc, message, strlen(message), 0);
        
        int num_of_masseg;
        //recv(socket_desc, , 2000 , 0);
        
        close(fd);
    }
    pthread_exit(0);
}

int main() {
    pthread_t thread_mass[THREAD_COUNT];
    
    for(int i = 0; i < THREAD_COUNT; ++i) {
        pthread_create(thread_mass + i, NULL, client_thread, NULL);
    }

    for(int i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(thread_mass[i], NULL);
    }
    return 0;
}
