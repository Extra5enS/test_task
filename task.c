#include<stdlib.h>
#include<stdio.h>
#include"task.h"

//#define atomic_increment(ptr) __atomic_fetch_add(ptr, 1, __ATOMIC_SEQ_CST)

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


void task_array_init(task_array* tarray, int size) {
    tarray -> array = calloc(size, sizeof(task*));
    tarray -> start = 0;
    tarray -> end = 0;
    tarray -> space = size;
    tarray -> size = 0;
    sem_init(&tarray -> wsem, 0, 0);
    sem_init(&tarray -> rsem, 0, size);
    pthread_mutex_init(&tarray -> mutex, NULL);
}

void task_array_push(task_array* tarray, task* t) {
    sem_wait(&tarray -> rsem); // Do we have place to write task?;

    tarray -> array[tarray -> end % tarray -> space] = t;
    tarray -> end++;
    tarray -> size++;
    
    sem_post(&tarray -> wsem); // Now we have one more task for workers;
}

void task_array_pop(task_array* tarray) {
    sem_wait(&tarray -> wsem); // Do we have tasks?
    pthread_mutex_lock(&tarray -> mutex);
    
    free(tarray -> array[tarray -> start % tarray -> space]);
    tarray -> array[tarray -> start % tarray -> space] = NULL;
    tarray -> start++;
    tarray -> size--;

    pthread_mutex_unlock(&tarray -> mutex);
    sem_post(&tarray -> rsem); // Now me have one more place fpr new task;
}

task* task_array_get(task_array* tarray) {
    task* res_task = NULL;
    sem_wait(&tarray -> wsem); // Do we have tasks?
    pthread_mutex_lock(&tarray -> mutex);

    res_task = tarray -> array[tarray -> start % tarray -> space];
    tarray -> array[tarray -> start % tarray -> space] = NULL;
    tarray -> start++;
    tarray -> size--;
    
    pthread_mutex_unlock(&tarray -> mutex);
    sem_post(&tarray -> rsem); // Now me have one more place fpr new task;
    
    return res_task;
}

void task_array_free(task_array* tarray) {
    free(tarray -> array);
}
