#include <errno.h>
#include "client-server.h"
#include "task.h"

#define BUF_SIZE 100

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
int semid;

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
            int msglen = 0;
            if((msglen = recv(client_sockets[i], client_message, ALL_SIZE, MSG_DONTWAIT)) > 0) {
                /* recv don't guarantee that a message of the specified length will be received.
                 * So it is necessary to wait for the remainder of the message in this case. */
                if(msglen != ALL_SIZE) {
                    recv(client_sockets[i], client_message + msglen, ALL_SIZE - msglen, MSG_WAITALL);
                }
                task* client_task = task_init(client_sockets[i], client_message); 
                task_array_push(&tarray, client_task);
                client_message = calloc(ALL_SIZE + 1, 1);
            }
        }
    }
    free(client_message);
    pthread_exit(0);
}


void* worker() {
    for(;;) {
        task* my_task = task_array_get(&tarray);
        sem_operation(semid, my_task -> thread_num, -1 * my_task -> message_num); 

        write(files_info[my_task -> thread_num].fd, skip_head(my_task -> client_message), MESSAGE_SIZE);
        fsync(files_info[my_task -> thread_num].fd);
       
        if(send(my_task -> client_socket, &my_task -> message_num, sizeof(int), 0) == -1) {
            perror(strerror(errno));
            exit(-1); 
        }

        sem_operation(semid, my_task -> thread_num, my_task -> message_num + 1);
        
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
    task_array_init(&tarray, BUF_SIZE); 
    semid = semget(IPC_PRIVATE, THREAD_COUNT, IPC_CREAT|IPC_EXCL|0600);

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
    task_array_free(&tarray);
    semctl(semid, 0, IPC_RMID);
    return 0;
}

