#ifndef UTIL
#define UTIL

#define END -128

typedef struct index_location{
    int index;
    char *path;
} index_location;

typedef struct index_location_stack{
    int cur;
    int len;
    index_location *content;
} index_location_stack;

index_location_stack *create_stack(int size);
index_location pop(index_location_stack *ils);
void push(index_location_stack *ils, index_location il);
int empty(index_location_stack *ils);
void clr_stack(index_location_stack *ils);
char **sort_files(char **files, int num_items);
char **merge(char **f1, int l1, char **f2, int l2);
char *lowercase(char *s);
char * get_full_path(char *s1, char *s2);
int *search_file(char **files, int num_items, char *name, int start_index,
        int reverse);
int cmpc_lower(char c1, char c2);
index_location *search_file_recur(char *path, char *name);
void sf_recur(char *path, char *name);
int int_len(int num);

#endif
