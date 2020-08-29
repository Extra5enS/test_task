
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


typedef struct {
    int client_socket;
    char * client_message;
} task;

task* global_task;

#define atomic_exchange(ptr, old_value_ptr, new_value) \
     while(!__atomic_compare_exchange_n(ptr, old_value_ptr, new_value, 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))

void* receiver() {
    int socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    int addrlen = sizeof(struct sockaddr_in);
	int new_socket;
    struct sockaddr_in server, client;
    
    server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8888 );
   
    bind(socket_desc,(struct sockaddr *)&server , sizeof(server));

    listen(socket_desc, THREAD_COUNT * 10);
    // start to revc new message from client; 
    while((new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&addrlen))){
        char * client_message = calloc(MESSAGE_SIZE, 1); 
        recv(new_socket, client_message, MESSAGE_SIZE, 0);
        task* client_task = calloc(1, sizeof(task));
        client_task -> client_socket = new_socket;
        client_task -> client_message = client_message;

        task* null_ptr = NULL;
        atomic_exchange(&global_task, &null_ptr, client_task);
    }
    pthread_exit(0);
}


void* worker() {
    while(1) {
        /* */
    }
    pthread_exit(0);
}

int main() {
    global_task = NULL;
    pthread_t worker_thread_mass[THREAD_COUNT];
    pthread_t receiver_thread;

    pthread_create(&receiver_thread, NULL, receiver, NULL);
    for(int i = 0; i < THREAD_COUNT / 3; ++i) {
        pthread_create(&worker_thread_mass[i], NULL, worker, NULL);
    }

    pthread_join(receiver_thread, NULL);
    for(int i = 0; i < THREAD_COUNT / 3; ++i) {
        pthread_join(worker_thread_mass[i], NULL);
    }
    return 0;

    return 0;
}

