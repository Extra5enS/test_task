#include "client-server.h"
#include <errno.h>
#include <fcntl.h>

#define SERVER_ADDR "0.0.0.0"
#define FILE_NAME "test_message"
/*
 * Changing the capacity of a pipe
       F_SETPIPE_SZ (int; since Linux 2.6.35)
              Change the capacity of the pipe referred to by fd to be at least arg bytes.  An unprivileged process can adjust the pipe capacity to any value between the system page size and the
              limit defined in /proc/sys/fs/pipe-max-size (see proc(5)).  Attempts to set the pipe capacity below the page size are silently rounded up to the page size.  Attempts by an unpriv‐
              ileged process to set the pipe capacity above the limit in /proc/sys/fs/pipe-max-size yield the error EPERM; a privileged process (CAP_SYS_RESOURCE) can override the limit.

              When allocating the buffer for the pipe, the kernel may use a capacity larger than arg, if that is convenient for the implementation.  (In the current implementation, the  alloca‐
              tion is the next higher power-of-two page-size multiple of the requested size.)  The actual capacity (in bytes) that is set is returned as the function result.

              Attempting to set the pipe capacity smaller than the amount of buffer space currently used to store data produces the error EBUSY.

       F_GETPIPE_SZ (void; since Linux 2.6.35)
              Return (as the function result) the capacity of the pipe referred to by fd.

 *
 */


typedef struct {
    int num;
    char * text;
} message_t;

typedef struct {
    char** array;
    int start;
    int end;
    int space;
} string_array;

void string_array_init(string_array* sarray, int size) {
    sarray -> array = calloc(size, sizeof(char*));
    sarray -> start = 0;
    sarray -> end = 0;
    sarray -> space = size;
}

void string_array_add(string_array* sarray, char* message) {
    sarray -> array[sarray -> end % sarray -> space] = message;
    sarray -> end++;
}

void string_array_delete(string_array* sarray) {
    free(sarray -> array[sarray -> start % sarray -> space]);
    sarray -> array[sarray -> start % sarray -> space] = NULL;
    sarray -> start++;
}

void string_array_free(string_array* sarray) {
    free(sarray -> array);
}

int string_array_size(string_array* sarray) {
    return sarray -> end - sarray -> start;
}

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

void creat_message(char * space_for_message, int thread_num, int message_num) {
    sprintf(space_for_message, "%d %d", thread_num, message_num);        
    for(int j = strlen(space_for_message); j < HEAD_SIZE; ++j) {
        space_for_message[j] = ' ';
    }
    int fd = open(FILE_NAME, O_RDONLY);  
    read(fd, skip_head(space_for_message), MESSAGE_SIZE); 
    close(fd);
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
    string_array_init(&sarray, 10);
    if(server_connection_init(&socket_desc, &server) < 0) {
        perror("Connection erron\n");
        exit(-1);
    }

    for(int i = 0; i < SEND_COUNT; i++) {
        char* message = calloc(MESSAGE_SIZE + HEAD_SIZE, 1); 
        creat_message(message, my_num, i);
        
        if(send(socket_desc, message, strlen(message), 0) == -1){
            perror(strerror(errno));
            exit(-1); 
        }
        string_array_add(&sarray, message);
        
        int num;
        do {
            num = -1;
            recv(socket_desc, &num, 4, 0);
            if(num != -1) {
                string_array_delete(&sarray);
            }
        } while(num != -1);
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
