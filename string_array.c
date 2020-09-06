#include<stdlib.h>
#include"string_array.h"

void string_array_init(string_array* sarray, int size) {
    sarray -> array = calloc(size, sizeof(char*));
    sarray -> start = 0;
    sarray -> end = 0;
    sarray -> space = size;
}

int string_array_size(string_array* sarray) {
    if(sarray -> end >= sarray -> start) {
        return sarray -> end - sarray -> start;    
    } else {
        return sarray -> space - sarray -> start + sarray -> end;
    }
}

void string_array_add(string_array* sarray, char* message) {
    if(string_array_size(sarray) == sarray -> space) {
        char** new_array = calloc(sarray -> space * 2, sizeof(char*));
        int iter_for_new_array = 0;
        for(int i = sarray -> start; i != sarray -> end; ++i) {
            new_array[iter_for_new_array++] = sarray -> array[i % sarray -> space];
        }
        sarray -> start = 0;
        sarray -> end = sarray -> space;
        sarray -> space *= 2;
        free(sarray -> array);
        sarray -> array = new_array;
    }
    sarray -> array[sarray -> end % sarray -> space] = message;
    sarray -> end++;
}

void string_array_delete(string_array* sarray) {
    free(sarray -> array[sarray -> start % sarray -> space]);
    sarray -> array[sarray -> start % sarray -> space] = NULL;
    sarray -> start++;
}

void string_array_free(string_array* sarray) {
    free(sarray -> array);
}
