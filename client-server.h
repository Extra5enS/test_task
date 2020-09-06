#ifndef CLIENT_SERVER
#define CLIENT_SERVER
#include<pthread.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

#include<sys/types.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#include <sys/stat.h>
#include <fcntl.h>

#define THREAD_COUNT 64 
#define MESSAGE_SIZE (1024*8)
#define HEAD_SIZE 20
#define SEND_COUNT 5000//000
#define SERVER_PORT 8888

#define atomic_load(ptr) __atomic_load_n(ptr, __ATOMIC_SEQ_CST)
#define atomic_store(ptr, new_value)    __atomic_store_n(ptr, new_value, __ATOMIC_SEQ_CST)
#define atomic_cas(ptr, old_value_ptr, new_value) __atomic_compare_exchange_n(ptr, old_value_ptr, new_value, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#define atomic_increment(ptr) __atomic_fetch_add(ptr, 1, __ATOMIC_SEQ_CST)
#define skip_head(prt) (prt + HEAD_SIZE)

#endif
