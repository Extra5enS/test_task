#ifndef STRING_ARRAY
#define STRING_ARRAY

typedef struct {
    char** array;
    int start;
    int end;
    int space;
    int size;
} string_array;

void string_array_init(string_array* sarray, int size);
int string_array_add(string_array* sarray, char* message);
void string_array_delete(string_array* sarray);
void string_array_free(string_array* sarray);

#endif
