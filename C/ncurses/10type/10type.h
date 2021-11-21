#include <time.h>
#include <stdlib.h>

#ifndef TYPE_10
#define TYPE_10
#define MAX_WORDS 100
#define ESCAPE 27
#define CONFIG_LOCATION "/.10type/config"
#define WORDLIST_LOCATION "/.10type/wordlists/"
#define TMP_FILE_LOCATION "/.10type/tmp"
#define STAT_FILE_LOCATION "/.10type/stats"
#define MAX_PARAM_SIZE 25
#define MAX_VALUE_SIZE 100
#define MAX_LIST_NAME_LENGTH 30
#define MAX_LIST_LENGTH 100 
#define MAX_TIME_LENGTH 10
#define MAX_MODE_LENGTH 10
#define MAX_RANDOM_LENGTH 10
#define MAX_COLOR_LENGTH 4
#define MAX_OPT_SIZE 50
#define MICROSEC_T 1000000
#define EPSILON 0.01
#define PROGRESS_C "▒"
#define TIMED_COLOR COLOR_YELLOW
#define CONSUME_COLOR COLOR_CYAN
#define THUMB_F COLOR_GREEN
#define INDEX_F COLOR_BLUE
#define MIDDLE_F COLOR_MAGENTA
#define RING_F COLOR_YELLOW
#define LITTLE_F COLOR_RED
#define PT_XSTART 6
#define PT_XLENGTH max_x - 12
#define WL_YSTART 5 
#define WL_XSTART 0
#define WL_YLENGTH max_y - 8 

struct wordinfo_t{
    int *indices;
    int length;
};

struct wordlists_t{
    int num_entries;
    int *num_written;
    double *best_wpm;
    double *avg_wpm;
    double *avg_acc;
    char **lists;
};

struct error_t{
    int words;
    int chars;
    int chars_in_words;
    int thumb;
    int l_little;
    int l_ring;
    int l_middle;
    int l_index;
    int r_little;
    int r_ring;
    int r_middle;
    int r_index;
};

struct total_t{
    int words;
    int chars;
    int thumb;
    int l_little;
    int l_ring;
    int l_middle;
    int l_index;
    int r_little;
    int r_ring;
    int r_middle;
    int r_index;
};

struct indices_t{
    int count;
    int *content;
};

void location_init();
void set_params();
void def_colors();
void def_windows(int len);
void undraw_full_keyboard();
void draw_full_keyboard();
void draw_lrwin();
void draw_fingerwin();
void draw_keyboard();
void color_keyboard_key(char c);
void update_wordlist(char *filename);
void set_wordlength(FILE *fp);
void set_wordlist(char *filename);
void sort_wordlists(struct wordlists_t *wordlists);
void update_wordlist_stats(struct wordlists_t *wordlists, int index,
        double wpm, double acc);
void get_wordlist_stats(struct wordlists_t *wordlists, int index);
void print_words(struct indices_t *indices, int row);
void refresh_screen();
void update_progress_step();
void write_to_file(char *filename);
void change_params();
void repeat();
void print_options(WINDOW *win, char options[][MAX_OPT_SIZE], int start_y,
        int start_x, int len, short color);
void pause_time();
void register_keystroke(char c, int err, struct total_t *total,
        struct error_t *errors);
void update_progress();
void check(struct indices_t *indices, struct total_t *total,
        struct error_t *errors);
void play_timed_random(struct total_t *total, struct error_t *errors);
void play_timed_iterative(int words, struct total_t *total,
        struct error_t *errors);
void play_consume_random(int words, struct total_t *total,
        struct error_t *errors);
void play_consume_iterative(int word, struct total_t *total,
        struct error_t *errors);
void statwin_add_blanks(double num);
void print_stats(struct total_t *total, struct error_t *errors);
void play(int words, time_t secs);
void cleanup();
void *clockword_thread(void *arg);
int find_list(FILE *fp, char *list);
int timed();
int iterative_play();
int end_criterium_reached();
int get_len(int val);
char *get_wordlist_name(char *list);
struct indices_t *search_for_list(struct wordlists_t *wordlists, char *str);
struct wordlists_t *get_avail_wordlists(char *folder);
struct indices_t *get_rand_wordindices();
struct indices_t *get_iter_wordindices(int word_count);
struct indices_t *get_consume_rand_wordindices(struct wordinfo_t *info);

#endif
