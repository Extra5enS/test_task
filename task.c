#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include"task.h"

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

pthread_mutex_t wmutex = PTHREAD_MUTEX_INITIALIZER;

void task_array_init(task_array* tarray, int size) {
    tarray -> array = calloc(size, sizeof(task*));
    tarray -> start = 0;
    tarray -> end = 0;
    tarray -> space = size;
    tarray -> size = 0;
}

int task_array_add(task_array* tarray, task* t) {
    if(tarray -> size == tarray -> space) {
        return 0;
    }
    tarray -> array[tarray -> end % tarray -> space] = t;
    tarray -> end++;
    tarray -> size++;
    return 1;
}

void task_array_delete(task_array* tarray) {
    pthread_mutex_lock(&wmutex);
    if(tarray -> size != tarray -> space) {
        free(tarray -> array[tarray -> start % tarray -> space]);
        tarray -> array[tarray -> start % tarray -> space] = NULL;
        tarray -> start++;
        tarray -> size--;
    }
    pthread_mutex_unlock(&wmutex);
}

task* task_array_get(task_array* tarray) {
    task* res_task = NULL;
    pthread_mutex_lock(&wmutex);
    if(tarray -> size != tarray -> space) {
        res_task = tarray -> array[tarray -> start % tarray -> space];
        tarray -> array[tarray -> start % tarray -> space] = NULL;
        tarray -> start++;
        tarray -> size--;
    }
    pthread_mutex_unlock(&wmutex);
    return res_task;
}

void task_array_free(task_array* tarray) {
    free(tarray -> array);
}
