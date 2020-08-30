
/* Add logic with socket
 * 
 * For a seekable file (i.e., one to which lseek(2) may be applied, for example, a regular file) writing takes place at the file offset, \
 * and the file offset is incremented by the number  of bytes  actually  written.   
 * If  the file was open(2)ed with O_APPEND, the file offset is first set to the end of the file before writing.  
 * The adjustment of the file offset and the write operation are performed as an atomic step.
 *
 * A successful return from write() does not make any guarantee that data has been committed to disk.  
 * On some filesystems, including NFS, it does not even guarantee that space has success‐fully  been reserved for the data.  
 * In this case, some errors might be delayed until a future write(2), fsync(2), or even close(2).  
 * The only way to be sure is to call fsync(2) after you are done writing all your data.*/

#include "client-server.h"
#include <errno.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int client_socket;
    int message_num;
    int thread_num;
    char* client_message;
} task;

task* task_init(int client_socket, char* client_message) {
    task* client_task = calloc(1, sizeof(task));
    sscanf(client_message, "%d %d", &(client_task -> thread_num), &(client_task -> message_num));
    client_task -> client_socket = client_socket;
    client_task -> client_message = client_message;
    
    return client_task;
}

void task_free(task* client_task) {
    free(client_task -> client_message);
    free(client_task);
}


typedef struct {
    int next_message_num;
    int fd;
} file_info;

// when you finish to use file_info? you must write close(file_info.fd); to close file!
file_info file_info_init(char* filename) {
    file_info fileinfo;
    fileinfo.fd = open(filename, O_APPEND|O_CREAT|O_WRONLY, S_IWRITE|S_IREAD);
    fileinfo.next_message_num = 0;
    return fileinfo;
} 

task* global_task;
file_info fd_for_client_threads[THREAD_COUNT];

// Done
void* receiver() {
    int socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    int addrlen = sizeof(struct sockaddr_in);
	int client_sockets[THREAD_COUNT];
    struct sockaddr_in server;
    struct sockaddr_in clients[9];
    
    server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( SERVER_PORT );
   
    bind(socket_desc,(struct sockaddr *)&server , sizeof(server));
    listen(socket_desc, THREAD_COUNT);
    
    // start to revc new message from client;
    
    for(int i = 0; i < THREAD_COUNT; ++i) {
        client_sockets[i] = accept(socket_desc,NULL /*(struct sockaddr *)&clients[i]*/,0 /*(socklen_t*)&addrlen*/);
    }

    char* client_message = calloc(MESSAGE_SIZE + HEAD_SIZE, 1); 
    for(;;) {
        for(int i = 0; i < THREAD_COUNT; ++i) {
            if(recv(client_sockets[i], client_message, MESSAGE_SIZE + HEAD_SIZE, 0 /*MSG_DONTWAIT*/) > 0) {
                task* client_task = task_init(client_sockets[i], client_message);
                task* null_ptr = NULL;
                
                while(atomic_load(&global_task) || !atomic_cas(&global_task, &null_ptr, client_task)) {
                    usleep(10);
                    //printf("%p\n", global_task);
                }

                client_message = calloc(MESSAGE_SIZE + HEAD_SIZE, 1);       
            } else {
                perror(strerror(errno));
            }
        }
    }
    free(client_message);
    pthread_exit(0);
}


void* worker() {
    for(;;) {
        task* my_task;
        do {
            usleep(10);
            my_task = atomic_load(&global_task);
        } while(!my_task || !atomic_cas(&global_task, &my_task, NULL));
        
        printf("%d %d\n", my_task->thread_num, my_task -> message_num); 
        while(my_task -> message_num != atomic_load(&fd_for_client_threads[my_task -> thread_num].next_message_num)) {
            usleep(10);
        }
        write(fd_for_client_threads[my_task -> thread_num].fd, my_task -> client_message + HEAD_SIZE, strlen(my_task -> client_message + HEAD_SIZE));
        fsync(fd_for_client_threads[my_task -> thread_num].fd);
        
        send(my_task -> client_socket, "Done", 5, 0);
    
        int next_message_num = fd_for_client_threads[my_task -> thread_num].next_message_num + 1;
        atomic_store(&fd_for_client_threads[my_task -> thread_num].next_message_num, next_message_num);
        task_free(my_task);
    }
    pthread_exit(0);
}


int main() {
    global_task = NULL;
    pthread_t worker_thread_mass[THREAD_COUNT];
    pthread_t receiver_thread;

    for(int i = 0; i < THREAD_COUNT; ++i) {
        char filename[3];
        sprintf(filename, "%d", i);
        fd_for_client_threads[i] = file_info_init(filename);
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

