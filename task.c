#include<stdlib.h>
#include<stdio.h>
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


void task_array_init(task_array* tarray) {
    tarray -> array = NULL;
    pthread_mutex_init(&tarray -> mutex, NULL);
    pthread_mutex_init(&tarray -> rm, NULL);
    pthread_mutex_init(&tarray -> wm, NULL);
    pthread_mutex_lock(&tarray -> wm);
}

void task_array_push(task_array* tarray, task* t) {
    pthread_mutex_lock(&tarray -> rm);
    tarray -> array = t;
    pthread_mutex_unlock(&tarray -> wm);   
}

void task_array_pop(task_array* tarray) {
    pthread_mutex_lock(&tarray -> mutex);
    pthread_mutex_lock(&tarray -> wm);
    
    free(tarray -> array);
    tarray -> array = NULL;

    pthread_mutex_unlock(&tarray -> rm);
    pthread_mutex_unlock(&tarray -> mutex);
}

task* task_array_get(task_array* tarray) {
    task* res_task = NULL;
    pthread_mutex_lock(&tarray -> mutex);
    pthread_mutex_lock(&tarray -> wm);

    res_task = tarray -> array;
    tarray -> array = NULL;
    
    pthread_mutex_unlock(&tarray -> mutex);
    pthread_mutex_unlock(&tarray -> rm);
    
    return res_task;
}

void task_array_free(task_array* tarray) {
    free(tarray -> array);
}
