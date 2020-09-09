#ifndef TASK
#define TASK
#include<pthread.h>
#include<semaphore.h>

typedef struct {
    int client_socket;
    int message_num;
    int thread_num;
    char* client_message;
} task;

task* task_init(int client_socket, char* client_message);
void task_free(task* client_task);

typedef struct {
    task** array;
    int start;
    int end;
    int space;
    int size;
    sem_t wsem, rsem;
    pthread_mutex_t mutex;
} task_array;

void task_array_init(task_array* tarray, int size);
void task_array_push(task_array* tarray, task* message);
void task_array_pop(task_array* tarray);
void task_array_free(task_array* tarray);
task* task_array_get(task_array* tarray);

#endif
