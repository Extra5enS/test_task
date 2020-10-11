#ifndef TASK
#define TASK
#include<pthread.h>

typedef struct {
    int client_socket;
    int message_num;
    int thread_num;
    char* client_message;
} task;

task* task_init(int client_socket, char* client_message);
void task_free(task* client_task);


typedef struct {
    task** queue;
    size_t ri, wi;
    pthread_cond_t rcond, wcond;
} task_queue;

void task_queue_init(task_queue* tq);
void task_queue_free(task_queue* tq);

size_t task_queue_size(task_queue* tq);
int task_queue_empty(task_queue* tq);
int task_queue_full(task_queue* tq);

int task_queue_push(task_queue* tq, task* message);
int task_queue_pop(task_queue* tq);
task* task_queue_get(task_queue* tq);


typedef struct {
    task_queue* array;
    size_t size;
} nqueue;

void nqueue_init(nqueue* nq, size_t work_num);
void nqueue_free(nqueue* nq);

size_t nqueue_find_place(nqueue* nq);

void nqueue_push(nqueue* nq, task* message);
void nqueue_pop(nqueue* nq, size_t my_num);
task* nqueue_get(nqueue* nq, size_t my_num);

#endif
