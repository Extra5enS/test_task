#include <errno.h>
#include <fcntl.h>
#include "client-server.h"
#include "string_array.h"

#define SERVER_ADDR "0.0.0.0"
#define FILE_NAME "test_message"
#define MAX_BUF_SIZE 20

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
    if(server_connection_init(&socket_desc, &server) < 0) {
        perror("Connection erron\n");
        exit(-1);
    }
     
    int recv_buf_size = 0;
    unsigned int size_of_int = sizeof(int);
    getsockopt(socket_desc, SOL_SOCKET, SO_SNDBUF, &recv_buf_size, &size_of_int); // return value greater then wmem_max

    string_array sarray;
    string_array_init(&sarray, recv_buf_size / ALL_SIZE);
    
    for(int i = 0; i < SEND_COUNT; i++) {
        char* message = calloc(ALL_SIZE + 1, 1); 
        int fd = open(FILE_NAME, O_RDONLY);  
        creat_message(fd, message, my_num, i);
        close(fd);
        
        if(send(socket_desc, message, ALL_SIZE, 0) == -1){
            perror(strerror(errno));
            exit(-1); 
        }

        int num = 0;
        do {
            int flage = sarray.size == sarray.space ? 0 : MSG_DONTWAIT;
            num = -1;
            recv(socket_desc, &num, sizeof(int), flage);
            if(num != -1) {
                string_array_delete(&sarray, num);
            }
        } while(num != -1 && sarray.size != 0);
        string_array_add(&sarray, i, message);
    }
    while(sarray.size != 0) {
        int num = -1;
        recv(socket_desc, &num, 4, 0);
        string_array_delete(&sarray, num);
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

