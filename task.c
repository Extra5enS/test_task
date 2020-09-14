#include<stdlib.h>
#include<stdio.h>
#include"task.h"


void count_lock_init(count_lock* cl, int start_count) {
    cl -> counter = start_count;
    pthread_mutex_init(&cl -> main_lock, NULL);
    pthread_mutex_init(&cl -> save_lock, NULL);
}

void count_lock_up(count_lock* cl) {
    pthread_mutex_lock(&cl -> save_lock);
    cl -> counter++;
    if(cl -> counter - 1 == 0) {
        pthread_mutex_unlock(&cl -> main_lock); 
    }
    pthread_mutex_unlock(&cl -> save_lock);
}

void count_lock_down(count_lock* cl) {
    int can_go = 0;
    while(!can_go) {
        pthread_mutex_lock(&cl -> main_lock);
        
        pthread_mutex_lock(&cl -> save_lock);
        if(cl -> counter != 0) {
            cl -> counter--;
            pthread_mutex_unlock(&cl -> main_lock);
            can_go = 1;
        }
        pthread_mutex_unlock(&cl -> save_lock);
    }
}

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


void task_array_init(task_array* tarray, int space) {
    tarray -> array = calloc(space, sizeof(task*));
    tarray -> start = 0;
    tarray -> end = 0;
    tarray -> size = 0;
    tarray -> space = space;
    pthread_mutex_init(&tarray -> mutex, NULL);
    count_lock_init(&tarray -> rlock, space);
    count_lock_init(&tarray -> wlock, 0);
}

void task_array_push(task_array* tarray, task* t) {
    count_lock_down(&tarray -> rlock);

    tarray -> array[tarray -> end % tarray -> space] = t;
    tarray -> end++;
    tarray -> size++;
    
    count_lock_up(&tarray -> wlock);
}

void task_array_pop(task_array* tarray) {
    count_lock_down(&tarray -> wlock);
    pthread_mutex_lock(&tarray -> mutex);
    
    free(tarray -> array[tarray -> start % tarray -> space]);
    tarray -> array[tarray -> start % tarray -> space] = NULL;
    tarray -> start++;
    tarray -> size--;

    pthread_mutex_unlock(&tarray -> mutex);
    count_lock_up(&tarray -> rlock);
}

task* task_array_get(task_array* tarray) {
    task* res_task = NULL;
    count_lock_down(&tarray -> wlock);
    pthread_mutex_lock(&tarray -> mutex);

    res_task = tarray -> array[tarray -> start % tarray -> space];
    tarray -> array[tarray -> start % tarray -> space] = NULL;
    tarray -> start++;
    tarray -> size--;
    
    pthread_mutex_unlock(&tarray -> mutex);
    count_lock_up(&tarray -> rlock);
    
    return res_task;
}

void task_array_free(task_array* tarray) {
    free(tarray -> array);
}
