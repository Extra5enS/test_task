#include <errno.h>
#include <sys/time.h>
#include "client-server.h"
#include "task.h"

#define MESSAGE_ARRAY_SIZE 200

typedef struct {
    int* message_array;
    int array_size;
    int message_count;
    int sended_num;
    int fd;
    pthread_mutex_t mutex;
} file_info;

// when you finish to use file_info? you must write close(file_info.fd); to close file!
file_info file_info_init(char* filename) {
    file_info fileinfo;
    fileinfo.fd = open(filename, O_APPEND|O_CREAT|O_WRONLY, __S_IWRITE|__S_IREAD);
    fileinfo.message_array = calloc(MESSAGE_ARRAY_SIZE, sizeof(int));
    fileinfo.message_count = 0;
    fileinfo.array_size = 0;
    fileinfo.sended_num = 0;
    pthread_mutex_init(&fileinfo.mutex, NULL);
    return fileinfo;
} 

void file_info_write(file_info* fi, task* t) {
    int array_size = 0;
    int sended_num = 0;
    int* message_array;
    
    pthread_mutex_lock(&fi -> mutex);
    
    fi -> message_array[fi -> array_size] = t -> message_num; 
    fi -> array_size++;
    write(fi -> fd, skip_head(t -> client_message), MESSAGE_SIZE);
    array_size = fi -> array_size;
    if(array_size == MESSAGE_ARRAY_SIZE) {
        message_array = calloc(MESSAGE_ARRAY_SIZE, sizeof(int));
        memcpy(message_array, fi -> message_array, MESSAGE_ARRAY_SIZE * sizeof(int));
        fi -> array_size = 0;
        sended_num = ++fi -> sended_num;
    }
    pthread_mutex_unlock(&fi -> mutex);
    
    if(array_size == MESSAGE_ARRAY_SIZE) {
        sync_file_range(fi -> fd, (sended_num - 1) * MESSAGE_ARRAY_SIZE * MESSAGE_SIZE, 
                       sended_num * MESSAGE_ARRAY_SIZE * MESSAGE_SIZE,
                       SYNC_FILE_RANGE_WAIT_BEFORE | SYNC_FILE_RANGE_WRITE);
        for(size_t i = 0; i < MESSAGE_ARRAY_SIZE; ++i) {
            send(t -> client_socket, &message_array[i], sizeof(int), 0); 
        }
        free(message_array);
    }
    if(sended_num * MESSAGE_ARRAY_SIZE == SEND_COUNT) {
        close(fi -> fd);
    }
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
nqueue nq;

void* receiver() {
    int socket_desc;
	int client_sockets[THREAD_COUNT];
    struct sockaddr_in server;
    server_init(&socket_desc, &server);
    fd_set fds;
    int max_fd = 0;
    FD_ZERO(&fds);
    for(int i = 0; i < THREAD_COUNT; ++i) {
        client_sockets[i] = accept(socket_desc, NULL, 0);
        FD_SET(client_sockets[i], &fds);
        if(max_fd < client_sockets[i]) {
            max_fd = client_sockets[i];
        }
    }
    
    char* client_message = calloc(ALL_SIZE + 1, 1); 
    for(;;) {
        select(max_fd + 1, &fds, NULL, NULL, NULL);
        for(int i = 0; i < THREAD_COUNT; ++i) {
            if(!FD_ISSET(client_sockets[i], &fds)) {
               FD_SET(client_sockets[i], &fds);
               continue;
            }
            int msglen = 0;
            // try to use select or epoll
            if((msglen = recv(client_sockets[i], client_message, ALL_SIZE, MSG_DONTWAIT)) > 0) {
                /* recv don't guarantee that a message of the specified length will be received.
                 * So it is necessary to wait for the remainder of the message in this case. */
                while(msglen != ALL_SIZE) {
                    int len = recv(client_sockets[i], client_message + msglen, ALL_SIZE - msglen, 0);
                    msglen += len;
                }
                task* client_task = task_init(client_sockets[i], client_message); 
                nqueue_push(&nq, client_task);
                client_message = calloc(ALL_SIZE + 1, 1);

            }
        }
    }
    free(client_message);
    pthread_exit(0);
}


void* worker(void* num) {
    int my_num = *(int*)num;
    for(;;) {
        task* my_task = nqueue_get(&nq, my_num);
        file_info_write(&files_info[my_task -> thread_num], my_task); 
        task_free(my_task);
    }
    pthread_exit(0);
}


int main() {
    global_task = NULL;
    pthread_t worker_thread_mass[THREAD_COUNT];
    int num_array[THREAD_COUNT];
    pthread_t receiver_thread;
    nqueue_init(&nq, THREAD_COUNT / 3); 

    for(int i = 0; i < THREAD_COUNT; ++i) {
        char filename[3];
        sprintf(filename, "%d", i);
        files_info[i] = file_info_init(filename);
    } 

    pthread_create(&receiver_thread, NULL, receiver, NULL);
    for(int i = 0; i < THREAD_COUNT / 3; ++i) {
        num_array[i] = i;
        pthread_create(&worker_thread_mass[i], NULL, worker, &num_array[i]);
    }

    pthread_join(receiver_thread, NULL);
    for(int i = 0; i < THREAD_COUNT / 3; ++i) {
        pthread_join(worker_thread_mass[i], NULL);
    }
    nqueue_free(&nq);
    return 0;
}

