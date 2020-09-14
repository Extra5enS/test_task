#ifndef TASK
#define TASK
#include<pthread.h>

typedef struct {
    int counter;
    pthread_mutex_t main_lock;
    pthread_mutex_t save_lock;
} count_lock;

void count_lock_init(count_lock* cl, int start_count);
void count_lock_up(count_lock* cl);
void count_lock_down(count_lock* cl);

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
    pthread_mutex_t mutex;
    count_lock rlock, wlock;
} task_array;

void task_array_init(task_array* tarray, int space);
void task_array_push(task_array* tarray, task* message);
void task_array_pop(task_array* tarray);
void task_array_free(task_array* tarray);
task* task_array_get(task_array* tarray);

#endif
