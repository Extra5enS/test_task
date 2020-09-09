#include<stdlib.h>
#include<stdio.h>
#include"task.h"

#define R_SEM 0
#define W_SEM 1

int sem_operation(int semid, int semname, int n) {
    struct sembuf buf = {semname, n, 0};
    return semop(semid, &buf, 1);
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
    tarray -> semid = semget(IPC_PRIVATE, 2, IPC_CREAT|IPC_EXCL|0600);
    sem_operation(tarray -> semid, R_SEM, space);
    pthread_mutex_init(&tarray -> mutex, NULL);
}

void task_array_push(task_array* tarray, task* t) {
    sem_operation(tarray -> semid, R_SEM, -1); // Do we have place to write task?

    tarray -> array[tarray -> end % tarray -> space] = t;
    tarray -> end++;
    tarray -> size++;
    
    sem_operation(tarray -> semid, W_SEM, 1); // Now we have one more task for workers
}

void task_array_pop(task_array* tarray) {
    sem_operation(tarray -> semid, W_SEM, -1); // Do we have tasks?
    pthread_mutex_lock(&tarray -> mutex);
    
    free(tarray -> array[tarray -> start % tarray -> space]);
    tarray -> array[tarray -> start % tarray -> space] = NULL;
    tarray -> start++;
    tarray -> size--;

    pthread_mutex_unlock(&tarray -> mutex);
    sem_operation(tarray -> semid, R_SEM, 1); // Now me have one more place for new task;
}

task* task_array_get(task_array* tarray) {
    task* res_task = NULL;
    sem_operation(tarray -> semid, W_SEM, -1); // Do we have tasks?
    pthread_mutex_lock(&tarray -> mutex);

    res_task = tarray -> array[tarray -> start % tarray -> space];
    tarray -> array[tarray -> start % tarray -> space] = NULL;
    tarray -> start++;
    tarray -> size--;
    
    pthread_mutex_unlock(&tarray -> mutex);
    sem_operation(tarray -> semid, R_SEM, 1); // Now me have one more place for new task;
    
    return res_task;
}

void task_array_free(task_array* tarray) {
    free(tarray -> array);
}
