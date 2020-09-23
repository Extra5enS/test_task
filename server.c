#include <errno.h>
#include <sys/time.h>
#include "client-server.h"
#include "task.h"

#define BUF_SIZE 5000

typedef struct {
    int next_message_num;
    int fd;
    pthread_mutex_t mutex;
} file_info;

// when you finish to use file_info? you must write close(file_info.fd); to close file!
file_info file_info_init(char* filename) {
    file_info fileinfo;
    fileinfo.fd = open(filename, O_APPEND|O_CREAT|O_WRONLY, __S_IWRITE|__S_IREAD);
    fileinfo.next_message_num = 0;
    pthread_mutex_init(&fileinfo.mutex, NULL);
    return fileinfo;
} 

void file_info_lock(file_info* fi, int wait_value) {
    for(;;) {
        pthread_mutex_lock(&fi -> mutex);
        if(fi -> next_message_num == wait_value) {
            break;
        } else {
            pthread_mutex_unlock(&fi -> mutex);
        }
    }
}

void file_info_unlock(file_info* fi) {
    fi -> next_message_num++;
    pthread_mutex_unlock(&fi -> mutex);
}

void file_info_write(file_info* fi, char* message) {
    write(fi -> fd, message, MESSAGE_SIZE);
    fsync(fi -> fd);
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
            msglen = recv(client_sockets[i], client_message, ALL_SIZE, MSG_DONTWAIT);
            /* recv don't guarantee that a message of the specified length will be received.
             * So it is necessary to wait for the remainder of the message in this case. */
            while(msglen != ALL_SIZE) {
                int len = recv(client_sockets[i], client_message + msglen, ALL_SIZE - msglen, 0);
                msglen += len;
            }
            task* client_task = task_init(client_sockets[i], client_message); 
            task_array_push(&tarray, client_task);
            client_message = calloc(ALL_SIZE + 1, 1);
        }
    }
    free(client_message);
    pthread_exit(0);
}


void* worker() {
    for(;;) {
        struct timeval start_time, task_get_time, write_time, send_time;
        gettimeofday(&start_time, NULL); //

        task* my_task = task_array_get(&tarray);
        gettimeofday(&task_get_time, NULL); // 

        /* Next operation is need to wait writing of previous message. */
        file_info_lock(&files_info[my_task -> thread_num], my_task -> message_num); 
        file_info_write(&files_info[my_task -> thread_num], skip_head(my_task -> client_message));
        
        gettimeofday(&write_time, NULL); //
         
        if(send(my_task -> client_socket, &my_task -> message_num, sizeof(int), 0) == -1) {
            perror(strerror(errno));
            exit(-1); 
        } 
        /* Now we write that next message have number = message_num + 1 */
        file_info_unlock(&files_info[my_task -> thread_num]); 
        gettimeofday(&send_time, NULL); //
        if(files_info[my_task -> thread_num].next_message_num == SEND_COUNT) {
            close(files_info[my_task -> thread_num].fd);
        }
        task_free(my_task);
        printf("task get: %f; write: %f; send: %f\n", 
                task_get_time.tv_sec - start_time.tv_sec + (task_get_time.tv_usec - start_time.tv_usec) / 1000000.,
                write_time.tv_sec - task_get_time.tv_sec + (write_time.tv_usec - task_get_time.tv_usec) / 1000000.,
                send_time.tv_sec - write_time.tv_sec + (send_time.tv_usec - write_time.tv_usec) / 1000000.
                );
    }
    pthread_exit(0);
}


int main() {
    global_task = NULL;
    pthread_t worker_thread_mass[THREAD_COUNT];
    pthread_t receiver_thread;
    task_array_init(&tarray, BUF_SIZE); 

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
    return 0;
}

