#ifndef TASK
#define TASK
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
} task_array;

void task_array_init(task_array* tarray, int size);
int task_array_size(task_array* tarray);
void task_array_add(task_array* tarray, task* message);
void task_array_delete(task_array* tarray);
void task_array_free(task_array* tarray);
task* task_array_get(task_array* tarray);

#endif
