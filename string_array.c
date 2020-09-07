#include<stdlib.h>
#include"string_array.h"

void string_array_init(string_array* sarray, int size) {
    sarray -> array = calloc(size, sizeof(char*));
    sarray -> start = 0;
    sarray -> end = 0;
    sarray -> space = size;
    sarray -> size = 0;
}

int string_array_add(string_array* sarray, char* message) {
    if(sarray -> size == sarray -> space) {
        return 0;
    }
    sarray -> array[sarray -> end % sarray -> space] = message;
    sarray -> end++;
    sarray -> size++;
    return 1;
}

void string_array_delete(string_array* sarray) {
    free(sarray -> array[sarray -> start % sarray -> space]);
    sarray -> array[sarray -> start % sarray -> space] = NULL;
    sarray -> start++;
    sarray -> size--;
}

void string_array_free(string_array* sarray) {
    free(sarray -> array);
}
