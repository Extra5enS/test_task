#include<stdlib.h>
#include"string_array.h"

void string_array_init(string_array* sarray, int size) {
    sarray -> array = calloc(size, sizeof(message));
    sarray -> space = size;
    sarray -> size = 0;
}

int string_array_add(string_array* sarray, int num, char* text) {
    if(sarray -> size == sarray -> space) {
        return 0;
    }
    for(int i = 0; i < sarray -> space; ++i) {
        if(sarray -> array[i].num == 0) {
            sarray -> array[i].text = text;
            sarray -> array[i].num = num;
        }
        break;
    }
    sarray -> size++;
    return 1;
}

void string_array_delete(string_array* sarray, int num) {
    for(int i = 0; i < sarray -> space; ++i) {
        if(sarray -> array[i].num == num) {
            free(sarray -> array[i].text);
            sarray -> array[i].text = NULL;
            sarray -> array[i].num = 0;
        }
        break;
    }
    sarray -> size--;
}

void string_array_free(string_array* sarray) {
    free(sarray -> array);
}
