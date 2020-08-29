#include<stdio.h>
#include"client-server.h"

#define SEND_COUNT 5000000

typedef struct {
    int num;
    char * text;
} message_t;

/*
 *  Working prototype without buf and nonblock recv;
 */
void *client_thread() {
    /* Here must be 5'000'000 messages about 8k every one
     * But here we will send one messag 5'000'000 times */
    int socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	struct sockaddr_in server;
    
    server.sin_addr.s_addr = inet_addr("0.0.0.0"); // addres of server 
	server.sin_family = AF_INET;
	server.sin_port = htons( 80 );
    
    connect(socket_desc , (struct sockaddr*)&server , sizeof(server));
    
    for(int i = 0; i < SEND_COUNT; i++) {
        char message[MESSAGE_SIZE]; 
        
        int fd = open("test_message", O_RDONLY); 
        // open 5'000'000 times to simulate 5'000'000 diferent massages 
        read(fd, message, MESSAGE_SIZE);
        
        send(socket_desc, message, strlen(message), 0);
        
        int num_of_masseg = 0;
        recv(socket_desc, &num_of_masseg, sizeof(num_of_masseg), 0);
        
        close(fd);
    }
    close(socket_desc);
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
