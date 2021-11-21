#include <curses.h>

#include <X11/Xutil.h>
#include "util.h"

#ifndef CURSBROWSE
#define CURSBROWSE

#define BIN_LOCATION_1 "/bin"
#define BIN_LOCATION_2 "/usr/local/bin"
#define CONFIG_PATH "/home/mark/.cursebrowserc"
#define CONFIG_LINE_LENGTH 256
#define WINDOW_NAME_LENGTH 50
#define RESIZE 410
#define BSP KEY_BACKSPACE
#define ESC 27
#define DEFAULT_STACK_SIZE 100
#define CMD_SIZE 50
#define FULL 1
#define REDUCED 2
#define SIMPLE 3
#define ERR_INPUT -1
#define ERR_DIR -2
#define ERR_PERMISSION -3
#define ERR_CONFIG -4
#define ERR_START_PATH -5
#define ERR_RECUR -6
#define ERR_SEARCH -7
#define ERR_COMMAND -8
#define ERR_PREV_COMMAND -9
#define ERR_PATH -10
#define ERR_MOUSE -11
#define NO_MATCH -12
#define MOUSE6_PRESSED 0b100000000010000000000000000
#define MOUSE7_PRESSED 0b100001000000000000000000000

typedef struct dir_files{
    char **files;
    char **perm;
    int *is_dir;
    int *is_link;
    gid_t *gid;
    uid_t *uid;
    off_t *size;
    struct timespec *last_access;
    int max_uid_len;
    int max_gid_len;
    int num_items;
} dir_files;

typedef struct file_matches{
    char ***matches;
    int num_items;
} file_matches;

file_matches *get_matches(char *path);
dir_files *get_files(char *path, int full);
char *get_perm(mode_t mode);
int test_path(char *path);
void cleanup_df(dir_files *df, int full);
void cleanup_fm(file_matches *fm);
void cleanup_ils(index_location_stack *ils);
void show_err(int n);
void show_info(int n);
int is_index(int *indices, int i);
void update_display(dir_files *df, char *path, int index, int print_ind,
        int has_indices, int *indices);
char *update_path(char *path, char *f, int is_dir, int is_link);
void def_colors(void);
void *run_command(void *arg);
char *create_command(char *fmt, ...);
int match(file_matches *fm, char *file_name, char *path);
int is_match(file_matches *fm, char *str);
char *get_start_path(char **argv, int supplied);
char *get_command(void);
int check_cmd(char *cmd);
Window get_window(void);
void set_window_name(Window w, char *name);
void toggle_index(int *indices, int *indices_init, int *n, int index, int
        num_items);

#endif
