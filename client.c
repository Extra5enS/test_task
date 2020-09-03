#include "client-server.h"
#include <errno.h>
#include <fcntl.h>

#define SERVER_ADDR "0.0.0.0"
#define FILE_NAME "test_message"
#define MAX_BUF_SIZE 1

typedef struct {
    int need_to_delete;
    int sd;
    int is_close;
} delete_node;

typedef struct {
    delete_node* array;
    int size;
    int sd_count;
} dn_info;

void dn_info_init(dn_info* dn, int size) {
    dn -> array = calloc(size, sizeof(delete_node));
    dn -> size = size;
    dn -> sd_count = 0;
    for(int i = 0; i < size; i++) {
        dn -> array[i].need_to_delete = 0;
        dn -> array[i].sd = 0;
        dn -> array[i].is_close = 0;
    }
}

delete_node* dn_info_at(dn_info* dn, int num) {
    return dn -> array + num;
}   

void dn_info_free(dn_info* dn) {
    free(dn -> array);
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

void creat_message(int fd, char* space_for_message, int thread_num, int message_num) {
    sprintf(space_for_message, "%d %d", thread_num, message_num);        
    for(int j = strlen(space_for_message); j < HEAD_SIZE; ++j) {
        space_for_message[j] = ' ';
    }
    read(fd, skip_head(space_for_message), MESSAGE_SIZE); 
}

dn_info dninfo; 

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
    dninfo.array[my_num].sd = socket_desc;
    atomic_increment(&dninfo.sd_count);

    for(int i = 0; i < SEND_COUNT; i++) {
        char* message = calloc(MESSAGE_SIZE + HEAD_SIZE, 1); 
        int fd = open(FILE_NAME, O_RDONLY);  
        creat_message(fd, message, my_num, i);
        close(fd);
        
        if(send(socket_desc, message, strlen(message), 0) == -1){
            perror(strerror(errno));
            exit(-1); 
        }
        string_array_add(&sarray, message);
        
        int num;
        do {
            num = -1;
           /*
            * logic to work with income segment
            */
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
    dn_info_init(&dninfo, THREAD_COUNT);
    for(int i = 0; i < THREAD_COUNT; ++i) {
        num_mass[i] = i;
        pthread_create(&thread_mass[i], NULL, client_thread, &num_mass[i]);
    }
    while(dninfo.sd_count != THREAD_COUNT);

    while(dninfo.sd_count != 0) {
        for(int i = 0; i < THREAD_COUNT; ++i) {
            int num;
            recv(dninfo.array[i].sd, &num, sizeof(int), 0);
            atomic_increment(&dninfo.array[i].need_to_delete);
        }
    }

    for(int i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(thread_mass[i], NULL);
    }
    printf("Done\n");
    return 0;
}
