#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "cursbrowse.h"
#include "util.h"
#include "check_file.h"

index_location *il;
int found;

index_location_stack *create_stack(int size)
{
    index_location_stack *ils;

    ils = malloc(sizeof(*ils));
    ils->content = malloc(sizeof(index_location) * size);
    ils->cur = 0;
    ils->len = size;

    return ils;
}

index_location pop(index_location_stack *ils)
{
    index_location il; 

    if(ils->cur == 0){
        il.index = -1;
        return il;
    }
    return ils->content[--ils->cur];
}

void push(index_location_stack *ils, index_location il)
{
    if(ils->cur == ils->len - 1){
        ils->len *= 2;
        ils->content = realloc(ils->content, sizeof(index_location) * ils->len);
    }
    ils->content[ils->cur++] = il;
}

int empty(index_location_stack *ils)
{
    return ils->cur == 0;
}

void clr_stack(index_location_stack *ils)
{
    for(ils->cur--; ils->cur >= 0; ils->cur--)
        free(ils->content[ils->cur].path);
    ils->cur++;
}

char **sort_files(char **files, int num_items)
{
    if(num_items == 1)
        return files;

    return merge(sort_files(files, num_items / 2), num_items / 2,
            sort_files(&(files[num_items / 2]), num_items - num_items / 2),
            num_items - num_items / 2);
}

char **merge(char **f1, int l1, char **f2, int l2)
{
    int n, i, j;
    char **merged, *low1, *low2;    

    merged = malloc((l1 + l2) * sizeof(char *));
    low1 = lowercase(f1[0]);
    low2 = lowercase(f2[0]);
    for(n = i = j = 0; i < l1 && j < l2; n++){
        if(!strcmp(low1, "..") || strcmp(low1, low2) <= 0){
            merged[n] = f1[i++];
            if(i < l1){
                free(low1);
                low1 = lowercase(f1[i]);
            }
        }else{
            merged[n] = f2[j++];
            if(j < l2){
                free(low2);
                low2 = lowercase(f2[j]);
            }
        }
    }
    free(low1);
    free(low2);
    if(i < l1){
        while(i < l1)
            merged[n++] = f1[i++];
    }else if(j < l2){
        while(j < l2)
            merged[n++] = f2[j++];
    }
    if(l2 > 1)
        free(f2);
    if(l1 > 1)
        free(f1);

    return merged;
}

char *lowercase(char *s)
{
    int i;
    char *low;
    
    low = malloc(strlen(s) + 1);
    for(i = 0; i < strlen(s); i++)
        low[i] = (s[i] > 64 && s[i] < 91 ? s[i] + ('a' - 'A') : s[i]);
    low[i] = '\0';

    return low;
}

int int_len(int num)
{
    int i;

    i = 0;
    if(num == 0)
        return 1;
    else if(num < 0){
        num *= -1;
        i++;
    }
    while(num > 0){
        num /= 10;
        i++;
    }

    return i;
}

char *get_full_path(char *s1, char *s2)
{
    char *full_path;

    full_path = malloc(strlen(s1) + strlen(s2) + 2);
    strcpy(full_path, s1);
    if(strcmp(s1, "/"))
      strcat(full_path, "/");
    strcat(full_path, s2);

    return full_path;
}

int *search_file(char **files, int num_items, char *name, int start_index,
        int reverse)
{
    int index, len, n, i, j, k, *indices; 

    len = strlen(name);
    indices = calloc(num_items + 1, sizeof(int));
    indices[0] = END;
    n = 0;
    for(i = 0, index = start_index; i < num_items; i++, index = (reverse ? index
                == 0 ? num_items - 1 : index - 1 : (index + 1) % num_items)){
        for(j = k = 0; k < len && j < strlen(files[index]); j++){
            if(cmpc_lower(files[index][j], name[k]))
                k++;
            else
                k = 0;
        }
        if(k == len){
            indices[n++] = index;
            indices[n] = END;
        }
    }
    
    return indices;
}

int cmpc_lower(char c1, char c2)
{
    if(c1 >= 'A' && c1 <= 'Z')
        c1 += ('a' - 'A');
    if(c2 >= 'A' && c2 <= 'Z')
        c2 += ('a' - 'A');
    return c1 == c2;
}

index_location *search_file_recur(char *path, char *name)
{
    dir_files *df;
    char *full_path;
    int i, *indices;

    found = 0;
    df = get_files(path, 0);
    indices = search_file(df->files, df->num_items, name, 0, 0);
    if(indices[0] != END){
        il = malloc(sizeof(*il));
        il->path = malloc(strlen(path) + 1);
        strcpy(il->path, path);
        il->index = indices[0];
        cleanup_df(df, 0);
        free(indices);
        return il;
    }
    free(indices);
    for(i = 0; i < df->num_items; i++){
        if(found){
            cleanup_df(df, 0);
            return il;
        }else if(strcmp(df->files[i], "..") && df->is_dir[i] && 
                !df->is_link[i]){
            full_path = get_full_path(path, df->files[i]);
            sf_recur(full_path, name);
        }
    }
    cleanup_df(df, 0);
    if(found)
        return il;
    return NULL;
}

void sf_recur(char *path, char *name)
{
    dir_files *df;
    char *full_path;
    int i, *indices;

    if(!test_path(path))
        free(path);
    else if(!found){
        df = get_files(path, 0);
        indices = search_file(df->files, df->num_items, name, 0, 0);
        if(indices[0] != END){
            il = malloc(sizeof(*il));
            il->path = malloc(strlen(path) + 1);
            strcpy(il->path, path);
            il->index = indices[0];
            cleanup_df(df, 0);
            free(indices);
            free(path);
            found = 1;
        }else{
            for(i = 0; !found && i < df->num_items; i++){
                if(strcmp(df->files[i], "..") && df->is_dir[i] && 
                        !df->is_link[i]){
                    full_path = get_full_path(path, df->files[i]);
                    sf_recur(full_path, name);
                }
            }
            cleanup_df(df, 0);
            free(indices);
            free(path);
        }
    }
}
