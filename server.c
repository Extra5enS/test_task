#include <errno.h>
#include "client-server.h"
#include "task.h"

#define RECV_MSG_NUM 8

typedef struct {
    int next_message_num;
    int fd;
} file_info;

// when you finish to use file_info? you must write close(file_info.fd); to close file!
file_info file_info_init(char* filename) {
    file_info fileinfo;
    fileinfo.fd = open(filename, O_APPEND|O_CREAT|O_WRONLY, __S_IWRITE|__S_IREAD);
    fileinfo.next_message_num = 0;
    return fileinfo;
} 

void server_init(int* socket_desc, struct sockaddr_in* server) {
    *socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    server -> sin_family = AF_INET;
	server -> sin_addr.s_addr = INADDR_ANY;
	server -> sin_port = htons( SERVER_PORT );
    
    bind(*socket_desc,(struct sockaddr *)server , sizeof(*server));
    listen(*socket_desc, THREAD_COUNT);
}

task* global_task;
file_info files_info[THREAD_COUNT];
task_array tarray;


void* receiver() {
    int socket_desc;
	int client_sockets[THREAD_COUNT];
    struct sockaddr_in server;
    server_init(&socket_desc, &server);
    for(int i = 0; i < THREAD_COUNT; ++i) {
        client_sockets[i] = accept(socket_desc, NULL, 0);
    }
 
    char* client_message = calloc(ALL_SIZE + 1, 1); 
    for(;;) {
        for(int i = 0; i < THREAD_COUNT; ++i) {
            recv(client_sockets[i], client_message, ALL_SIZE, 0/*MSG_WAITALL*/); // <==
            task* client_task = task_init(client_sockets[i], client_message); 
            task_array_add(&tarray, client_task);
            client_message = calloc(ALL_SIZE + 1, 1);                   
        }
    }
    free(client_message);
    pthread_exit(0);
}


void* worker() {
    for(;;) {
        task* my_task = NULL;
        my_task = task_array_get(&tarray);
        while(my_task -> message_num != atomic_load(&files_info[my_task -> thread_num].next_message_num)); 

        write(files_info[my_task -> thread_num].fd, skip_head(my_task -> client_message), strlen(skip_head(my_task -> client_message)));
        fsync(files_info[my_task -> thread_num].fd);
       
        if(send(my_task -> client_socket, &my_task -> message_num, sizeof(int), 0) == -1) {
            perror(strerror(errno));
            exit(-1); 
        }

        atomic_increment(&files_info[my_task -> thread_num].next_message_num);
        if(files_info[my_task -> thread_num].next_message_num == SEND_COUNT) {
            close(files_info[my_task -> thread_num].fd);
        }
        task_free(my_task);
    }
    pthread_exit(0);
}


int main() {
    global_task = NULL;
    pthread_t worker_thread_mass[THREAD_COUNT];
    pthread_t receiver_thread;
    task_array_init(&tarray, 1000);
    
    for(int i = 0; i < THREAD_COUNT; ++i) {
        char filename[3];
        sprintf(filename, "%d", i);
        files_info[i] = file_info_init(filename);
    } 

    pthread_create(&receiver_thread, NULL, receiver, NULL);
    for(int i = 0; i < THREAD_COUNT / 3; ++i) {
        pthread_create(&worker_thread_mass[i], NULL, worker, NULL);
    }

    pthread_join(receiver_thread, NULL);
    for(int i = 0; i < THREAD_COUNT / 3; ++i) {
        pthread_join(worker_thread_mass[i], NULL);
    }
    return 0;
}

