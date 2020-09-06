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
}

int task_array_size(task_array* tarray) {
    if(tarray -> end >= tarray -> start) {
        return tarray -> end - tarray -> start;    
    } else {
        return tarray -> space - tarray -> start + tarray -> end;
    }
}

void task_array_add(task_array* tarray, task* t) {
    pthread_mutex_lock(&wmutex);
    if(task_array_size(tarray) == tarray -> space) {
        task** new_array = calloc(tarray -> space * 2, sizeof(task*));
        int iter_for_new_array = 0;
        for(int i = tarray -> start; i != tarray -> end; ++i) {
            new_array[iter_for_new_array++] = tarray -> array[i % tarray -> space];
        }
        tarray -> start = 0;
        tarray -> end = tarray -> space;
        tarray -> space *= 2;
        free(tarray -> array);
        tarray -> array = new_array;
    }
    pthread_mutex_unlock(&wmutex);
    tarray -> array[tarray -> end % tarray -> space] = t;
    tarray -> end++;
}

void task_array_delete(task_array* tarray) {
    pthread_mutex_lock(&wmutex);
    if(tarray -> start != tarray -> end) {
        free(tarray -> array[tarray -> start % tarray -> space]);
        tarray -> array[tarray -> start % tarray -> space] = NULL;
        tarray -> start++;
    }
    pthread_mutex_unlock(&wmutex);
}

task* task_array_get(task_array* tarray) {
    task* res_task = NULL;
    pthread_mutex_lock(&wmutex);
    if(tarray -> start != tarray -> end) {
        res_task = tarray -> array[tarray -> start % tarray -> space];
        tarray -> array[tarray -> start % tarray -> space] = NULL;
        tarray -> start++;
    }
    pthread_mutex_unlock(&wmutex);
    return res_task;
}

void task_array_free(task_array* tarray) {
    free(tarray -> array);
}
