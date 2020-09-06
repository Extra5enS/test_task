#include <errno.h>
#include <fcntl.h>
#include "client-server.h"
#include "string_array.h"

#define SERVER_ADDR "0.0.0.0"
#define FILE_NAME "test_message"
#define MAX_BUF_SIZE 10

/*
 * Dont forget to close socket_desc!
 */
int server_connection_init(int* socket_desc, struct sockaddr_in* server) {
    *socket_desc = socket(AF_INET , SOCK_STREAM, 0); 
    server -> sin_addr.s_addr = inet_addr(SERVER_ADDR); // addres of server 
    server -> sin_family = AF_INET;
    server -> sin_port = htons(SERVER_PORT);
    return connect(*socket_desc , (struct sockaddr*)server , sizeof(*server)); 
}

void creat_message(int fd, char* space_for_message, int thread_num, int message_num) {
    sprintf(space_for_message, "%d %d", thread_num, message_num);        
    for(int j = strlen(space_for_message); j < HEAD_SIZE; ++j) {
        space_for_message[j] = ' ';
    }
    read(fd, skip_head(space_for_message), MESSAGE_SIZE); 
}

/*
 *  Working prototype without buf and nonblock recv;
 */
void *client_thread(void* arg) {
    /* Here must be 5'000'000 messages about 8k every one
     * But here we will send one messag 5'000'000 times */
    int my_num = *(int*)arg;
    int socket_desc;
	struct sockaddr_in server;
    string_array sarray;
    string_array_init(&sarray, MAX_BUF_SIZE);
    if(server_connection_init(&socket_desc, &server) < 0) {
        perror("Connection erron\n");
        exit(-1);
    }

    for(int i = 0; i < SEND_COUNT; i++) {
        char* message = calloc(MESSAGE_SIZE + HEAD_SIZE + 1, 1); 
        int fd = open(FILE_NAME, O_RDONLY);  
        creat_message(fd, message, my_num, i);
        close(fd);
        
        if(send(socket_desc, message, MESSAGE_SIZE + HEAD_SIZE, 0) == -1){
            perror(strerror(errno));
            exit(-1); 
        }

        string_array_add(&sarray, message);
        int num = -1;
        if(recv(socket_desc, &num, sizeof(int), MSG_DONTWAIT) > 0) {
            string_array_delete(&sarray);
        }
    }
    while(string_array_size(&sarray) != 0) {
        int num = -1;
        recv(socket_desc, &num, 4, 0);
        string_array_delete(&sarray);
    }
    string_array_free(&sarray);
    close(socket_desc);
    pthread_exit(0);
}


int main() {
    pthread_t thread_mass[THREAD_COUNT];
    int num_mass[THREAD_COUNT];
    for(int i = 0; i < THREAD_COUNT; ++i) {
        num_mass[i] = i;
        pthread_create(&thread_mass[i], NULL, client_thread, &num_mass[i]);
    }
    for(int i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(thread_mass[i], NULL);
    }
    printf("Done\n");
    return 0;
}
