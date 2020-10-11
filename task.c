#include<stdlib.h>
#include<stdio.h>
#include"task.h"

#define QUEUE_MAX_SIZE 200

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

void task_queue_init(task_queue* tq) {
    tq -> queue = calloc(QUEUE_MAX_SIZE, sizeof(task*));
    tq -> ri = 0;
    tq -> wi = 0;
}

void task_queue_free(task_queue* tq) {
    free(tq -> queue);
}

int task_queue_push(task_queue* tq, task* t) {
    while(task_queue_full(tq)) {
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&tq -> rcond, &mutex);
    }
    tq -> queue[tq -> ri] = t;
    tq -> ri = (tq -> ri + 1) % QUEUE_MAX_SIZE; 
    pthread_cond_signal(&tq -> wcond);
    return 1;
}

int task_queue_pop(task_queue* tq) {
    while(task_queue_empty(tq)) {
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&tq -> wcond, &mutex);
    }
    free(tq -> queue[tq -> wi]);
    tq -> queue[tq -> wi] = NULL;
    tq -> wi = (tq -> wi + 1) % QUEUE_MAX_SIZE;
    pthread_cond_signal(&tq -> rcond);
    return 1;
}

task* task_queue_get(task_queue* tq) {
    task* res_task = NULL;
    while(task_queue_empty(tq)) {
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&tq -> wcond, &mutex);
    }
    res_task = tq -> queue[tq -> wi];
    tq -> queue[tq -> wi] = NULL;
    tq -> wi = (tq -> wi + 1) % QUEUE_MAX_SIZE;
    pthread_cond_signal(&tq -> rcond);
    return res_task;
}

size_t task_queue_size(task_queue* tq) {
    size_t ri = tq -> ri; 
    size_t wi = tq -> wi; 
    return (ri >= wi)?(ri - wi):(QUEUE_MAX_SIZE - wi + ri);
}

int task_queue_empty(task_queue* tq) {
    return tq -> ri == tq -> wi;
}

int task_queue_full(task_queue* tq) {
    return (tq -> ri + 1) % QUEUE_MAX_SIZE == tq -> wi;
}

void nqueue_init(nqueue* nq, size_t work_num) {
    nq -> array = calloc(work_num, sizeof(task_queue));
    for(int i = 0; i < work_num; ++i) {
        task_queue_init(&nq -> array[i]);
    }
    nq -> size = work_num;
}

void nqueue_free(nqueue* nq) {
    free(nq -> array);
}

size_t nqueue_find_place(nqueue* nq) {
    size_t min_place = 0;
    for(int i = 0; i < nq -> size; ++i) {
        min_place = (task_queue_size(&nq -> array[i]) < task_queue_size(&nq -> array[min_place])) ? i : min_place;
    }
    //printf("min size is = %ld, place = %ld\n", task_queue_size(&nq -> array[min_place]), min_place);
    return min_place;
}


void nqueue_push(nqueue* nq, task* message) {
    size_t place = 0;
    place = nqueue_find_place(nq);
    task_queue_push(&nq -> array[place], message);
}

void nqueue_pop(nqueue* nq, size_t my_num) {    
    task_queue_pop(&nq -> array[my_num]);
}

task* nqueue_get(nqueue* nq, size_t my_num) {
    task* res_task = NULL;
    res_task = task_queue_get(&nq -> array[my_num]);
    return res_task;
}
