#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <curses.h>
#include <unistd.h>
#include <locale.h>
#include <dirent.h>
#include <pwd.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>

#include "10type.h"

WINDOW *titlewin, *typewin, *printwin, *clockwordwin, *infowin, *progresswin,
       *keyboardwin, *fingerwin, *lrwin, *optionwin, *statwin;

char **wordlist, *list, *conf_location, *list_location, *tmp_location,
     *stats_location, progress_char[5] = PROGRESS_C;

int *wordlength, type_x, type_y, print_x, print_y, max_x, max_y, init_words,
    end, forced_end, thread_init, list_index, cur_words, init_secs = 60,
    mode = 0, iterative = 1;

double wpm, best_wpm, progress_step, cur_secs, starttime, progress;

short consume_iterative_pair_f = 1, consume_iterative_pair_b = 2,
      consume_random_pair_f = 3, consume_random_pair_b = 4,
      timed_iterative_pair_f = 5, timed_iterative_pair_b = 6,
      timed_random_pair_f = 7, timed_random_pair_b = 8,
      thumb_pair_f = 9, thumb_pair_b = 10,
      index_left_pair_f = 11, index_left_pair_b = 12,
      index_right_pair_f = 13, index_right_pair_b = 14,
      middle_left_pair_f = 15, middle_left_pair_b = 16,
      middle_right_pair_f = 17, middle_right_pair_b = 18,
      ring_left_pair_f = 19, ring_left_pair_b = 20,
      ring_right_pair_f = 21, ring_right_pair_b = 22,
      little_left_pair_f = 23, little_left_pair_b = 24,
      little_right_pair_f = 25, little_right_pair_b = 26,
      typewin_pair_f = 27, typewin_pair_b = 28,
      pausewin_pair_f = 29, pausewin_pair_b = 30,
      paramwin_pair_f = 31, paramwin_pair_b = 32,
      endwin_pair_f = 33, endwin_pair_b = 34,
      green_f = 35, red_f = 36, red_b = 37, num_written_pair_f = 38,
      best_wpm_pair_f = 39, avg_wpm_pair_f = 40, avg_acc_pair_f = 41;

short consume_iterative_color = CONSUME_COLOR,
      consume_random_color = CONSUME_COLOR,
      timed_iterative_color = TIMED_COLOR,
      timed_random_color = TIMED_COLOR,
      thumb_color = THUMB_F,
      index_left_color = INDEX_F,
      index_right_color = INDEX_F,
      middle_left_color = MIDDLE_F,
      middle_right_color = MIDDLE_F,
      ring_left_color = RING_F,
      ring_right_color = RING_F,
      little_left_color = LITTLE_F,
      little_right_color = LITTLE_F,
      typewin_color = COLOR_RED,
      pausewin_color = COLOR_GREEN,
      endwin_color = COLOR_BLUE,
      paramwin_color = COLOR_YELLOW,
      num_written_color = COLOR_WHITE,
      best_wpm_color = COLOR_YELLOW,
      avg_wpm_color = COLOR_RED,
      avg_acc_color = COLOR_GREEN;

struct wordlists_t *wordlists;
sem_t lock;
pthread_t thread;

void location_init(){
    char *homedir;

    if((homedir = getenv("HOME")) == NULL)
        homedir = getpwuid(getuid())->pw_dir;

    conf_location = malloc(strlen(homedir) + strlen(CONFIG_LOCATION) + 1);
    strcpy(conf_location, homedir);
    strcat(conf_location, CONFIG_LOCATION);

    list_location = malloc(strlen(homedir) + strlen(WORDLIST_LOCATION) + 1);
    strcpy(list_location, homedir);
    strcat(list_location, WORDLIST_LOCATION);

    tmp_location = malloc(strlen(homedir) + strlen(TMP_FILE_LOCATION) + 1);
    strcpy(tmp_location, homedir);
    strcat(tmp_location, TMP_FILE_LOCATION);

    stats_location = malloc(strlen(homedir) + strlen(STAT_FILE_LOCATION) + 1);
    strcpy(stats_location, homedir);
    strcat(stats_location, STAT_FILE_LOCATION);
}

void set_params(){
    FILE *configp;
    char c;
    char *param, *value;
    int first, i;

    param = malloc(MAX_PARAM_SIZE * sizeof(char));
    value = malloc(MAX_VALUE_SIZE * sizeof(char));
    list = malloc(MAX_LIST_LENGTH * sizeof(char));

    first = 1;
    strcpy(list, list_location);
    strcpy(list + strlen(list_location),
            "e_mostcommon_200\0");
    configp = fopen(conf_location, "r");
    if(configp != NULL){
        while((c = fgetc(configp)) != EOF){
            if(c == '\n')
                first = 1;
            else{
                if(first && c != ' ' && c != '#'){
                    i = 0;
                    memset(param, 0, sizeof(*param));
                    memset(value, 0, sizeof(*value));
                    param[0] = c;
                    fscanf(configp, "%[^=]", &param[1]);
                    c = fgetc(configp);
                    while(i < MAX_VALUE_SIZE -1 && (c = fgetc(configp))
                            != '\n' && c != EOF)
                        value[i++] = c;
                    value[i] = '\0';
                    if(strcmp(param, "mode") == 0){
                        if(strcmp(value, "timed") == 0)
                            mode = 1;
                        else if(strcmp(value, "consume") == 0)
                            mode = 0;
                    }else if(strcmp(param, "advance-mode") == 0){
                        if(strcmp(value, "random") == 0)
                            iterative = 0;
                        else if(strcmp(value, "iterative") == 0)
                            iterative = 1;
                    }else if(strcmp(param, "wordlist") == 0){
                        strcpy(list, value);
                    }else if(strcmp(param, "time") == 0){
                        init_secs = atoi(value);
                    }else if(strcmp(param, "consume_iterative_color") == 0){
                        consume_iterative_color = atoi(value);
                    }else if(strcmp(param, "consume_random_color") == 0){
                        consume_random_color = atoi(value);
                    }else if(strcmp(param, "timed_iterative_color") == 0){
                        timed_iterative_color = atoi(value);
                    }else if(strcmp(param, "timed_random_color") == 0){
                        timed_random_color = atoi(value);
                    }else if(strcmp(param, "thumb_color") == 0){
                        thumb_color = atoi(value);
                    }else if(strcmp(param, "index_left_color") == 0){
                        index_left_color = atoi(value);
                    }else if(strcmp(param, "index_right_color") == 0){
                        index_right_color = atoi(value);
                    }else if(strcmp(param, "middle_left_color") == 0){
                        middle_left_color = atoi(value);
                    }else if(strcmp(param, "middle_right_color") == 0){
                        middle_right_color = atoi(value);
                    }else if(strcmp(param, "ring_left_color") == 0){
                        ring_left_color = atoi(value);
                    }else if(strcmp(param, "ring_right_color") == 0){
                        ring_right_color = atoi(value);
                    }else if(strcmp(param, "little_left_color") == 0){
                        little_left_color = atoi(value);
                    }else if(strcmp(param, "little_right_color") == 0){
                        little_right_color = atoi(value);
                    }else if(strcmp(param, "pausewin_color") == 0){
                        pausewin_color = atoi(value);
                    }else if(strcmp(param, "typewin_color") == 0){
                        typewin_color = atoi(value);
                    }else if(strcmp(param, "endwin_color") == 0){
                        endwin_color = atoi(value);
                    }else if(strcmp(param, "paramwin_color") == 0){
                        paramwin_color = atoi(value);
                    }else if(strcmp(param, "num_written_color") == 0){
                        num_written_color = atoi(value);
                    }else if(strcmp(param, "best_wpm_color") == 0){
                        best_wpm_color = atoi(value);
                    }else if(strcmp(param, "avg_wpm_color") == 0){
                        avg_wpm_color = atoi(value);
                    }else if(strcmp(param, "avg_acc_color") == 0){
                        avg_acc_color = atoi(value);
                    }else if(strcmp(param, "progress_char") == 0){
                        strncpy(progress_char, value, 4);
                        progress_char[4] = '\0';
                    }else
                        first = 0;
                }else
                    first = 0;
            }
        }
    }
    free(param);
    free(value);
}

void def_colors(){
    init_pair(green_f, COLOR_GREEN, -1);
    init_pair(red_f, COLOR_RED, -1);
    init_pair(red_b, COLOR_BLACK, COLOR_RED);

    init_pair(typewin_pair_f, typewin_color, -1);
    init_pair(typewin_pair_b, COLOR_BLACK, typewin_color);
    init_pair(pausewin_pair_f, pausewin_color, -1);
    init_pair(pausewin_pair_b, COLOR_BLACK, pausewin_color);
    init_pair(paramwin_pair_f, paramwin_color, -1);
    init_pair(paramwin_pair_b, COLOR_BLACK, paramwin_color);
    init_pair(endwin_pair_f, endwin_color, -1);
    init_pair(endwin_pair_b, COLOR_BLACK, endwin_color);

    init_pair(num_written_pair_f, num_written_color, -1);
    init_pair(best_wpm_pair_f, best_wpm_color, -1);
    init_pair(avg_wpm_pair_f, avg_wpm_color, -1);
    init_pair(avg_acc_pair_f, avg_acc_color, -1);

    init_pair(consume_iterative_pair_f, consume_iterative_color, -1);
    init_pair(consume_iterative_pair_b, COLOR_BLACK, consume_iterative_color);
    init_pair(consume_random_pair_f, consume_random_color, -1);
    init_pair(consume_random_pair_b, COLOR_BLACK, consume_random_color);
    init_pair(timed_iterative_pair_f, timed_iterative_color, -1);
    init_pair(timed_iterative_pair_b, COLOR_BLACK, timed_iterative_color);
    init_pair(timed_random_pair_f, timed_random_color, -1);
    init_pair(timed_random_pair_b, COLOR_BLACK, timed_random_color);

    init_pair(thumb_pair_f, thumb_color, -1);
    init_pair(thumb_pair_b, COLOR_BLACK, thumb_color);

    init_pair(index_left_pair_f, index_left_color, -1);
    init_pair(index_left_pair_b, COLOR_BLACK, index_left_color);
    init_pair(index_right_pair_f, index_right_color, -1);
    init_pair(index_right_pair_b, COLOR_BLACK, index_right_color);

    init_pair(middle_left_pair_f, middle_left_color, -1);
    init_pair(middle_left_pair_b, COLOR_BLACK, middle_left_color);
    init_pair(middle_right_pair_f, middle_right_color, -1);
    init_pair(middle_right_pair_b, COLOR_BLACK, middle_right_color);

    init_pair(ring_left_pair_f, ring_left_color, -1);
    init_pair(ring_left_pair_b, COLOR_BLACK, ring_left_color);
    init_pair(ring_right_pair_f, ring_right_color, -1);
    init_pair(ring_right_pair_b, COLOR_BLACK, ring_right_color);

    init_pair(little_left_pair_f, little_left_color, -1);
    init_pair(little_left_pair_b, COLOR_BLACK, little_left_color);
    init_pair(little_right_pair_f, little_right_color, -1);
    init_pair(little_right_pair_b, COLOR_BLACK, little_right_color);
}

void def_windows(int len){
    progresswin = newwin(1, max_x, 1, 0);
    clockwordwin = newwin(1, 15, 0, max_x - 15);
    infowin = newwin(1, strlen("[CONSUME|ITERATIVE]"), 0, 0);
    wattr_set(infowin, A_BOLD, 0, NULL);
    titlewin = newwin(1, len, 0, max_x / 2 - len / 2);
    wattr_set(titlewin, A_BOLD, 0, NULL);
    optionwin = newwin(1, max_x, max_y - 1, 0);
    wattr_set(optionwin, A_NORMAL, typewin_pair_b, NULL);
    printwin = newwin(2, PT_XLENGTH, 4, PT_XSTART);
    typewin = newwin(max_y - 23, PT_XLENGTH + 1, 7, PT_XSTART);
    idlok(typewin, true);
    scrollok(typewin, true);
    keyboardwin = newwin(11, 61 , max_y - 14, (double)max_x / 2 - 30);
    wattr_set(keyboardwin, A_BOLD, 0, NULL);
    fingerwin = newwin(1, 65, max_y - 3, (double)max_x /2 - 32);
    wattr_set(fingerwin, A_BOLD, 0, NULL);
    lrwin = newwin(1, 59, max_y - 15, (double)max_x /2 - 29);
    wattr_set(lrwin, A_BOLD, 0, NULL);
}

void undraw_full_keyboard(){
    werase(fingerwin);
    wrefresh(fingerwin);
    werase(keyboardwin);
    wrefresh(keyboardwin);
    werase(lrwin);
    wrefresh(lrwin);
}

void draw_full_keyboard(){
    draw_lrwin();
    draw_keyboard();
    draw_fingerwin();
}

void draw_lrwin(){
    mvwhline(lrwin, 0, 0, 0, 59);
    mvwaddch(lrwin, 0, 27, '|');
    mvwaddstr(lrwin, 0, 12, "left");
    mvwaddstr(lrwin, 0, 43, "right");
    wrefresh(lrwin);
}

void draw_fingerwin(){
    wprintw(fingerwin, " Little  Ring  Middle  Index  Thumb  Index  Middle  "
            "Ring  Little ");
    mvwchgat(fingerwin, 0, 0, 8, A_NORMAL, little_left_pair_b, NULL); 
    mvwchgat(fingerwin, 0, 8, 6, A_NORMAL, ring_left_pair_b, NULL); 
    mvwchgat(fingerwin, 0, 14, 8, A_NORMAL, middle_left_pair_b, NULL); 
    mvwchgat(fingerwin, 0, 22, 7, A_NORMAL, index_left_pair_b, NULL); 
    mvwchgat(fingerwin, 0, 29, 7, A_NORMAL, thumb_pair_b, NULL); 
    mvwchgat(fingerwin, 0, 36, 7, A_NORMAL, index_right_pair_b, NULL); 
    mvwchgat(fingerwin, 0, 43, 8, A_NORMAL, middle_right_pair_b, NULL); 
    mvwchgat(fingerwin, 0, 51, 6, A_NORMAL, ring_right_pair_b, NULL); 
    mvwchgat(fingerwin, 0, 57, 8, A_NORMAL, little_right_pair_b, NULL); 
    wrefresh(fingerwin);
}

void draw_keyboard(){
    box(keyboardwin, 0, 0); 

    mvwhline(keyboardwin, 2, 1, 0, 59);
    mvwhline(keyboardwin, 4, 1, 0, 59);
    mvwhline(keyboardwin, 6, 1, 0, 59);
    mvwhline(keyboardwin, 8, 1, 0, 59);

    mvwaddch(keyboardwin, 2, 0, ACS_LTEE);
    mvwaddch(keyboardwin, 4, 0, ACS_LTEE);
    mvwaddch(keyboardwin, 6, 0, ACS_LTEE);
    mvwaddch(keyboardwin, 8, 0, ACS_LTEE);

    mvwaddch(keyboardwin, 2, 60, ACS_RTEE);
    mvwaddch(keyboardwin, 4, 60, ACS_RTEE);
    mvwaddch(keyboardwin, 6, 60, ACS_RTEE);
    mvwaddch(keyboardwin, 8, 60, ACS_RTEE);

    mvwvline(keyboardwin, 1, 4, 0, 1);
    mvwvline(keyboardwin, 1, 8, 0, 1);
    mvwvline(keyboardwin, 1, 12, 0, 1);
    mvwvline(keyboardwin, 1, 16, 0, 1);
    mvwvline(keyboardwin, 1, 20, 0, 1);
    mvwvline(keyboardwin, 1, 24, 0, 1);
    mvwvline(keyboardwin, 1, 28, 0, 1);
    mvwvline(keyboardwin, 1, 32, 0, 1);
    mvwvline(keyboardwin, 1, 36, 0, 1);
    mvwvline(keyboardwin, 1, 40, 0, 1);
    mvwvline(keyboardwin, 1, 44, 0, 1);
    mvwvline(keyboardwin, 1, 48, 0, 1);
    mvwvline(keyboardwin, 1, 52, 0, 1);
    mvwvline(keyboardwin, 1, 56, 0, 1);

    mvwaddch(keyboardwin, 0, 4, ACS_TTEE);
    mvwaddch(keyboardwin, 0, 8, ACS_TTEE);
    mvwaddch(keyboardwin, 0, 12, ACS_TTEE);
    mvwaddch(keyboardwin, 0, 16, ACS_TTEE);
    mvwaddch(keyboardwin, 0, 20, ACS_TTEE);
    mvwaddch(keyboardwin, 0, 24, ACS_TTEE);
    mvwaddch(keyboardwin, 0, 28, ACS_TTEE);
    mvwaddch(keyboardwin, 0, 32, ACS_TTEE);
    mvwaddch(keyboardwin, 0, 36, ACS_TTEE);
    mvwaddch(keyboardwin, 0, 40, ACS_TTEE);
    mvwaddch(keyboardwin, 0, 44, ACS_TTEE);
    mvwaddch(keyboardwin, 0, 48, ACS_TTEE);
    mvwaddch(keyboardwin, 0, 52, ACS_TTEE);
    mvwaddch(keyboardwin, 0, 56, ACS_TTEE);

    mvwaddch(keyboardwin, 2, 4, ACS_BTEE);
    mvwaddch(keyboardwin, 2, 8, ACS_BTEE);
    mvwaddch(keyboardwin, 2, 12, ACS_BTEE);
    mvwaddch(keyboardwin, 2, 16, ACS_BTEE);
    mvwaddch(keyboardwin, 2, 20, ACS_BTEE);
    mvwaddch(keyboardwin, 2, 24, ACS_BTEE);
    mvwaddch(keyboardwin, 2, 28, ACS_BTEE);
    mvwaddch(keyboardwin, 2, 32, ACS_BTEE);
    mvwaddch(keyboardwin, 2, 36, ACS_BTEE);
    mvwaddch(keyboardwin, 2, 40, ACS_BTEE);
    mvwaddch(keyboardwin, 2, 44, ACS_BTEE);
    mvwaddch(keyboardwin, 2, 48, ACS_BTEE);
    mvwaddch(keyboardwin, 2, 52, ACS_BTEE);
    mvwaddch(keyboardwin, 2, 56, ACS_BTEE);

    mvwvline(keyboardwin, 3, 6, 0, 1);
    mvwvline(keyboardwin, 3, 10, 0, 1);
    mvwvline(keyboardwin, 3, 14, 0, 1);
    mvwvline(keyboardwin, 3, 18, 0, 1);
    mvwvline(keyboardwin, 3, 22, 0, 1);
    mvwvline(keyboardwin, 3, 26, 0, 1);
    mvwvline(keyboardwin, 3, 30, 0, 1);
    mvwvline(keyboardwin, 3, 34, 0, 1);
    mvwvline(keyboardwin, 3, 38, 0, 1);
    mvwvline(keyboardwin, 3, 42, 0, 1);
    mvwvline(keyboardwin, 3, 46, 0, 1);
    mvwvline(keyboardwin, 3, 50, 0, 1);
    mvwvline(keyboardwin, 3, 54, 0, 1);

    mvwaddch(keyboardwin, 2, 6, ACS_TTEE);
    mvwaddch(keyboardwin, 2, 10, ACS_TTEE);
    mvwaddch(keyboardwin, 2, 14, ACS_TTEE);
    mvwaddch(keyboardwin, 2, 18, ACS_TTEE);
    mvwaddch(keyboardwin, 2, 22, ACS_TTEE);
    mvwaddch(keyboardwin, 2, 26, ACS_TTEE);
    mvwaddch(keyboardwin, 2, 30, ACS_TTEE);
    mvwaddch(keyboardwin, 2, 34, ACS_TTEE);
    mvwaddch(keyboardwin, 2, 38, ACS_TTEE);
    mvwaddch(keyboardwin, 2, 42, ACS_TTEE);
    mvwaddch(keyboardwin, 2, 46, ACS_TTEE);
    mvwaddch(keyboardwin, 2, 50, ACS_TTEE);
    mvwaddch(keyboardwin, 2, 54, ACS_TTEE);

    mvwaddch(keyboardwin, 4, 6, ACS_PLUS);
    mvwaddch(keyboardwin, 4, 10, ACS_PLUS);
    mvwaddch(keyboardwin, 4, 14, ACS_PLUS);
    mvwaddch(keyboardwin, 4, 18, ACS_PLUS);
    mvwaddch(keyboardwin, 4, 22, ACS_PLUS);
    mvwaddch(keyboardwin, 4, 26, ACS_PLUS);
    mvwaddch(keyboardwin, 4, 30, ACS_PLUS);
    mvwaddch(keyboardwin, 4, 34, ACS_PLUS);
    mvwaddch(keyboardwin, 4, 38, ACS_PLUS);
    mvwaddch(keyboardwin, 4, 42, ACS_PLUS);
    mvwaddch(keyboardwin, 4, 46, ACS_PLUS);
    mvwaddch(keyboardwin, 4, 50, ACS_PLUS);
    mvwaddch(keyboardwin, 4, 54, ACS_BTEE);

    mvwvline(keyboardwin, 5, 6, 0, 1);
    mvwvline(keyboardwin, 5, 10, 0, 1);
    mvwvline(keyboardwin, 5, 14, 0, 1);
    mvwvline(keyboardwin, 5, 18, 0, 1);
    mvwvline(keyboardwin, 5, 22, 0, 1);
    mvwvline(keyboardwin, 5, 26, 0, 1);
    mvwvline(keyboardwin, 5, 30, 0, 1);
    mvwvline(keyboardwin, 5, 34, 0, 1);
    mvwvline(keyboardwin, 5, 38, 0, 1);
    mvwvline(keyboardwin, 5, 42, 0, 1);
    mvwvline(keyboardwin, 5, 46, 0, 1);
    mvwvline(keyboardwin, 5, 50, 0, 1);

    mvwaddch(keyboardwin, 6, 6, ACS_BTEE);
    mvwaddch(keyboardwin, 6, 10, ACS_BTEE);
    mvwaddch(keyboardwin, 6, 14, ACS_BTEE);
    mvwaddch(keyboardwin, 6, 18, ACS_BTEE);
    mvwaddch(keyboardwin, 6, 22, ACS_BTEE);
    mvwaddch(keyboardwin, 6, 26, ACS_BTEE);
    mvwaddch(keyboardwin, 6, 30, ACS_BTEE);
    mvwaddch(keyboardwin, 6, 34, ACS_BTEE);
    mvwaddch(keyboardwin, 6, 38, ACS_BTEE);
    mvwaddch(keyboardwin, 6, 42, ACS_BTEE);
    mvwaddch(keyboardwin, 6, 46, ACS_BTEE);
    mvwaddch(keyboardwin, 6, 50, ACS_BTEE);

    mvwvline(keyboardwin, 7, 8, 0, 1);
    mvwvline(keyboardwin, 7, 12, 0, 1);
    mvwvline(keyboardwin, 7, 16, 0, 1);
    mvwvline(keyboardwin, 7, 20, 0, 1);
    mvwvline(keyboardwin, 7, 24, 0, 1);
    mvwvline(keyboardwin, 7, 28, 0, 1);
    mvwvline(keyboardwin, 7, 32, 0, 1);
    mvwvline(keyboardwin, 7, 36, 0, 1);
    mvwvline(keyboardwin, 7, 40, 0, 1);
    mvwvline(keyboardwin, 7, 44, 0, 1);
    mvwvline(keyboardwin, 7, 48, 0, 1);
    mvwvline(keyboardwin, 7, 56, 0, 1);

    mvwaddch(keyboardwin, 6, 8, ACS_TTEE);
    mvwaddch(keyboardwin, 6, 12, ACS_TTEE);
    mvwaddch(keyboardwin, 6, 16, ACS_TTEE);
    mvwaddch(keyboardwin, 6, 20, ACS_TTEE);
    mvwaddch(keyboardwin, 6, 24, ACS_TTEE);
    mvwaddch(keyboardwin, 6, 28, ACS_TTEE);
    mvwaddch(keyboardwin, 6, 32, ACS_TTEE);
    mvwaddch(keyboardwin, 6, 36, ACS_TTEE);
    mvwaddch(keyboardwin, 6, 40, ACS_TTEE);
    mvwaddch(keyboardwin, 6, 44, ACS_TTEE);
    mvwaddch(keyboardwin, 6, 48, ACS_TTEE);
    mvwaddch(keyboardwin, 6, 56, ACS_TTEE);

    mvwaddch(keyboardwin, 8, 8, ACS_BTEE);
    mvwaddch(keyboardwin, 8, 12, ACS_BTEE);
    mvwaddch(keyboardwin, 8, 20, ACS_BTEE);
    mvwaddch(keyboardwin, 8, 24, ACS_BTEE);
    mvwaddch(keyboardwin, 8, 28, ACS_BTEE);
    mvwaddch(keyboardwin, 8, 32, ACS_BTEE);
    mvwaddch(keyboardwin, 8, 36, ACS_BTEE);
    mvwaddch(keyboardwin, 8, 40, ACS_BTEE);
    mvwaddch(keyboardwin, 8, 56, ACS_BTEE);

    mvwaddch(keyboardwin, 8, 16, ACS_PLUS);
    mvwaddch(keyboardwin, 8, 44, ACS_PLUS);
    mvwaddch(keyboardwin, 8, 48, ACS_PLUS);

    mvwvline(keyboardwin, 9, 6, 0, 1);
    mvwvline(keyboardwin, 9, 10, 0, 1);
    mvwvline(keyboardwin, 9, 16, 0, 1);
    mvwvline(keyboardwin, 9, 38, 0, 1);
    mvwvline(keyboardwin, 9, 44, 0, 1);
    mvwvline(keyboardwin, 9, 48, 0, 1);

    mvwaddch(keyboardwin, 8, 6, ACS_TTEE);
    mvwaddch(keyboardwin, 8, 10, ACS_TTEE);
    mvwaddch(keyboardwin, 8, 38, ACS_TTEE);

    mvwaddch(keyboardwin, 10, 6, ACS_BTEE);
    mvwaddch(keyboardwin, 10, 10, ACS_BTEE);
    mvwaddch(keyboardwin, 10, 16, ACS_BTEE);
    mvwaddch(keyboardwin, 10, 38, ACS_BTEE);
    mvwaddch(keyboardwin, 10, 44, ACS_BTEE);
    mvwaddch(keyboardwin, 10, 48, ACS_BTEE);

    mvwaddch(keyboardwin, 1, 1, 'E');
    mvwaddch(keyboardwin, 1, 2, 's'); 
    mvwaddch(keyboardwin, 1, 3, 'c'); 
    mvwaddch(keyboardwin, 1, 6, '1');
    mvwaddch(keyboardwin, 1, 10, '2');
    mvwaddch(keyboardwin, 1, 14, '3');
    mvwaddch(keyboardwin, 1, 18, '4');
    mvwaddch(keyboardwin, 1, 22, '5');
    mvwaddch(keyboardwin, 1, 26, '6');
    mvwaddch(keyboardwin, 1, 30, '7');
    mvwaddch(keyboardwin, 1, 34, '8');
    mvwaddch(keyboardwin, 1, 38, '9');
    mvwaddch(keyboardwin, 1, 42, '0');
    mvwaddch(keyboardwin, 1, 46, '-');
    mvwaddch(keyboardwin, 1, 50, '=');
    mvwaddch(keyboardwin, 1, 54, '\\');
    mvwaddch(keyboardwin, 1, 58, '`');

    mvwaddch(keyboardwin, 3, 2, 'T');
    mvwaddch(keyboardwin, 3, 3, 'a');
    mvwaddch(keyboardwin, 3, 4, 'b');
    mvwaddch(keyboardwin, 3, 8, 'q');
    mvwaddch(keyboardwin, 3, 12, 'w');
    mvwaddch(keyboardwin, 3, 16, 'e');
    mvwaddch(keyboardwin, 3, 20, 'r');
    mvwaddch(keyboardwin, 3, 24, 't');
    mvwaddch(keyboardwin, 3, 28, 'y');
    mvwaddch(keyboardwin, 3, 32, 'u');
    mvwaddch(keyboardwin, 3, 36, 'i');
    mvwaddch(keyboardwin, 3, 40, 'o');
    mvwaddch(keyboardwin, 3, 44, 'p');
    mvwaddch(keyboardwin, 3, 48, '[');
    mvwaddch(keyboardwin, 3, 52, ']');
    mvwaddch(keyboardwin, 3, 56, 'B');
    mvwaddch(keyboardwin, 3, 57, 's');
    mvwaddch(keyboardwin, 3, 58, 'p');

    mvwaddch(keyboardwin, 5, 2, 'S');
    mvwaddch(keyboardwin, 5, 3, 't');
    mvwaddch(keyboardwin, 5, 4, 'r');
    mvwaddch(keyboardwin, 5, 8, 'a');
    mvwaddch(keyboardwin, 5, 12, 's');
    mvwaddch(keyboardwin, 5, 16, 'd');
    mvwaddch(keyboardwin, 5, 20, 'f');
    mvwaddch(keyboardwin, 5, 24, 'g');
    mvwaddch(keyboardwin, 5, 28, 'h');
    mvwaddch(keyboardwin, 5, 32, 'j');
    mvwaddch(keyboardwin, 5, 36, 'k');
    mvwaddch(keyboardwin, 5, 40, 'l');
    mvwaddch(keyboardwin, 5, 44, ';');
    mvwaddch(keyboardwin, 5, 48, '\'');
    mvwaddch(keyboardwin, 5, 53, 'E');
    mvwaddch(keyboardwin, 5, 54, 'n');
    mvwaddch(keyboardwin, 5, 55, 't');
    mvwaddch(keyboardwin, 5, 56, 'e');
    mvwaddch(keyboardwin, 5, 57, 'r');

    mvwaddch(keyboardwin, 7, 2, 'S');
    mvwaddch(keyboardwin, 7, 3, 'h');
    mvwaddch(keyboardwin, 7, 4, 'i');
    mvwaddch(keyboardwin, 7, 5, 'f');
    mvwaddch(keyboardwin, 7, 6, 't');
    mvwaddch(keyboardwin, 7, 10, 'z');
    mvwaddch(keyboardwin, 7, 14, 'x');
    mvwaddch(keyboardwin, 7, 18, 'c');
    mvwaddch(keyboardwin, 7, 22, 'v');
    mvwaddch(keyboardwin, 7, 26, 'b');
    mvwaddch(keyboardwin, 7, 30, 'n');
    mvwaddch(keyboardwin, 7, 34, 'm');
    mvwaddch(keyboardwin, 7, 38, ',');
    mvwaddch(keyboardwin, 7, 42, '.');
    mvwaddch(keyboardwin, 7, 46, '/');
    mvwaddch(keyboardwin, 7, 50, 'S');
    mvwaddch(keyboardwin, 7, 51, 'h');
    mvwaddch(keyboardwin, 7, 52, 'i');
    mvwaddch(keyboardwin, 7, 53, 'f');
    mvwaddch(keyboardwin, 7, 54, 't');
    mvwaddch(keyboardwin, 7, 57, 'F');
    mvwaddch(keyboardwin, 7, 58, 'n');
    mvwaddch(keyboardwin, 7, 59, 'c');

    mvwaddch(keyboardwin, 9, 7, 'A');
    mvwaddch(keyboardwin, 9, 8, 'l');
    mvwaddch(keyboardwin, 9, 9, 't');
    mvwaddch(keyboardwin, 9, 11, 'S');
    mvwaddch(keyboardwin, 9, 12, 'u');
    mvwaddch(keyboardwin, 9, 13, 'p');
    mvwaddch(keyboardwin, 9, 14, 'e');
    mvwaddch(keyboardwin, 9, 15, 'r');
    mvwaddch(keyboardwin, 9, 25, 'S');
    mvwaddch(keyboardwin, 9, 26, 'p');
    mvwaddch(keyboardwin, 9, 27, 'a');
    mvwaddch(keyboardwin, 9, 28, 'c');
    mvwaddch(keyboardwin, 9, 29, 'e');
    mvwaddch(keyboardwin, 9, 39, 'S');
    mvwaddch(keyboardwin, 9, 40, 'u');
    mvwaddch(keyboardwin, 9, 41, 'p');
    mvwaddch(keyboardwin, 9, 42, 'e');
    mvwaddch(keyboardwin, 9, 43, 'r');
    mvwaddch(keyboardwin, 9, 45, 'A');
    mvwaddch(keyboardwin, 9, 46, 'l');
    mvwaddch(keyboardwin, 9, 47, 't');

    wrefresh(keyboardwin);
}

void color_keyboard_key(char c){
    int y, x, off, color, y2, x2, off2, color2;

    y2 = 0;
    if(c > 47 && c < 58){
        if(c == 48)
            y = 1, x = 41, off = 3, color = ring_right_pair_b;
        else{
            color = c < 51 ? little_left_pair_b : c == 51 ? ring_left_pair_b :
                c == 52 ? middle_left_pair_b : c == 53 || c == 54 ? 
                index_left_pair_b : c == 55 || c == 56 ? index_right_pair_b :
                middle_right_pair_b;
            y = 1, x = 1 + (c % 48) * 4, off = 3;
        }
    }else 
        if(c > 64 && c < 91){
            if(c < 72 || (c > 80 && c < 85) || (c > 85 && c < 89) || c == 90)
                y2 = 7, x2 = 49, off2 = 7, color2 = little_right_pair_b;
            else
                y2 = 7, x2 = 1, off2 = 7, color2 = little_left_pair_b;
            c += 32;
        }switch(c){
            case ' ' : y = 9, x = 17, off = 21, color = thumb_pair_b;
                       break;
            case '\b': y = 3, x = 55, off = 5, color = little_right_pair_b;
                       break;
            case 'q' : y = 3, x = 7, off = 3, color = little_left_pair_b;
                       break;
            case 'w' : y = 3, x = 11, off = 3, color = ring_left_pair_b;
                       break;
            case 'e' : y = 3, x = 15, off = 3, color = middle_left_pair_b;
                       break;
            case 'r' : y = 3, x = 19, off = 3, color = index_left_pair_b;
                       break;
            case 't' : y = 3, x = 23, off = 3, color = index_left_pair_b;
                       break;
            case 'y' : y = 3, x = 27, off = 3, color = index_right_pair_b;
                       break;
            case 'u' : y = 3, x = 31, off = 3, color = index_right_pair_b;
                       break;
            case 'i' : y = 3, x = 35, off = 3, color = middle_right_pair_b;
                       break;
            case 'o' : y = 3, x = 39, off = 3, color = ring_right_pair_b;
                       break;
            case 'p' : y = 3, x = 43, off = 3, color = little_right_pair_b;
                       break;
            case 'a' : y = 5, x = 7, off = 3, color = little_left_pair_b;
                       break;
            case 's' : y = 5, x = 11, off = 3, color = ring_left_pair_b;
                       break;
            case 'd' : y = 5, x = 15, off = 3, color = middle_left_pair_b;
                       break;
            case 'f' : y = 5, x = 19, off = 3, color = index_left_pair_b;
                       break;
            case 'g' : y = 5, x = 23, off = 3, color = index_left_pair_b;
                       break;
            case 'h' : y = 5, x = 27, off = 3, color = index_right_pair_b;
                       break;
            case 'j' : y = 5, x = 31, off = 3, color = index_right_pair_b;
                       break;
            case 'k' : y = 5, x = 35, off = 3, color = middle_right_pair_b;
                       break;
            case 'l' : y = 5, x = 39, off = 3, color = ring_right_pair_b;
                       break;
            case 'z' : y = 7, x = 9, off = 3, color = little_left_pair_b;
                       break;
            case 'x' : y = 7, x = 13, off = 3, color = ring_left_pair_b;
                       break;
            case 'c' : y = 7, x = 17, off = 3, color = middle_left_pair_b;
                       break;
            case 'v' : y = 7, x = 21, off = 3, color = index_left_pair_b;
                       break;
            case 'b' : y = 7, x = 25, off = 3, color = index_left_pair_b;
                       break;
            case 'n' : y = 7, x = 29, off = 3, color = index_right_pair_b;
                       break;
            case 'm' : y = 7, x = 33, off = 3, color = index_right_pair_b;
                       break;
            case ESCAPE : y = 1, x = 1, off = 3, color = little_left_pair_b;
                          break;
            case '\t': y = 3, x = 1, off = 5, color = little_left_pair_b;
                       break;
            case '\n': y = 5, x = 51, off = 9, color = little_right_pair_b;
                       break;
            case ',' : y = 7, x = 37, off = 3, color = middle_right_pair_b;
                       break;
            case '<' : y = 7, x = 1, off = 7, color = little_left_pair_b;
                       y2 = 7, x2 = 37, off2 = 3, color2 = middle_right_pair_b;
                       break;
            case '.' : y = 7, x = 41, off = 3, color = ring_right_pair_b;
                       break;
            case '>' : y = 7, x = 1, off = 7, color = little_left_pair_b;
                       y2 = 7, x2 = 41, off2 = 3, color2 = ring_right_pair_b;
                       break;
            case '/' : y = 7, x = 45, off = 3, color = little_right_pair_b;
                       break;
            case '?' : y = 7, x = 1, off = 7, color = little_left_pair_b;
                       y2 = 7, x2 = 45, off2 = 3, color2 = little_right_pair_b;
                       break;
            case ';' : y = 5, x = 43, off = 3, color = little_right_pair_b;
                       break;
            case ':' : y = 7, x = 1, off = 7, color = little_left_pair_b;
                       y2 = 5, x2 = 43, off2 = 3, color2 = little_right_pair_b;
                       break;
            case '\'': y = 5, x = 47, off = 3, color = little_right_pair_b;
                       break;
            case '"' : y = 7, x = 1, off = 7, color = little_left_pair_b;
                       y2 = 5, x2 = 47, off2 = 3, color2 = little_right_pair_b;
                       break;
            case '[' : y = 3, x = 47, off = 3, color = little_right_pair_b;
                       break;
            case '{' : y = 7, x = 1, off = 7, color = little_left_pair_b;
                       y2 = 3, x2 = 47, off2 = 3, color2 = little_right_pair_b;
                       break;
            case ']' : y = 3, x = 51, off = 3, color = little_right_pair_b;
                       break;
            case '}' : y = 7, x = 1, off = 7, color = little_left_pair_b;
                       y2 = 3, x2 = 51, off2 = 3, color2 = little_right_pair_b;
                       break;
            case '!' : y = 7, x = 49, off = 7, color = little_right_pair_b;
                       y2 = 1, x2 = 5, off2 = 3, color2 = little_left_pair_b;
                       break;
            case '@' : y = 7, x = 49, off = 7, color = little_right_pair_b;
                       y2 = 1, x2 = 9, off2 = 3, color2 = little_left_pair_b;
                       break;
            case '#' : y = 7, x = 49, off = 7, color = little_right_pair_b;
                       y2 = 1, x2 = 13, off2 = 3, color2 = ring_left_pair_b;
                       break;
            case '$' : y = 7, x = 49, off = 7, color = little_right_pair_b;
                       y2 = 1, x2 = 17, off2 = 3, color2 = middle_left_pair_b;
                       break;
            case '%' : y = 7, x = 49, off = 7, color = little_right_pair_b;
                       y2 = 1, x2 = 21, off2 = 3, color2 = index_left_pair_b;
                       break;
            case '^' : y = 7, x = 49, off = 7, color = little_right_pair_b;
                       y2 = 1, x2 = 25, off2 = 3, color2 = index_left_pair_b;
                       break;
            case '&' : y = 7, x = 1, off = 7, color = little_left_pair_b;
                       y2 = 1, x2 = 29, off2 = 3, color2 = index_right_pair_b;
                       break;
            case '*' : y = 7, x = 1, off = 7, color = little_left_pair_b;
                       y2 = 1, x2 = 33, off2 = 3, color2 = index_right_pair_b;
                       break;
            case '(' : y = 7, x = 1, off = 7, color = little_left_pair_b;
                       y2 = 1, x2 = 37, off2 = 3, color2 = middle_right_pair_b;
                       break;
            case ')' : y = 7, x = 1, off = 7, color = little_left_pair_b;
                       y2 = 1, x2 = 41, off2 = 3, color2 = ring_right_pair_b;
                       break;
            case '-' : y = 1, x = 45, off = 3, color = ring_right_pair_b;
                       break;
            case '_' : y = 7, x = 1, off = 7, color = little_left_pair_b;
                       y2 = 1, x2 = 45, off2 = 3, color2 = ring_right_pair_b;
                       break;
            case '=' : y = 1, x = 49, off = 3, color = little_right_pair_b;
                       break;
            case '+' : y = 7, x = 1, off = 7, color = little_left_pair_b;
                       y2 = 1, x2 = 49, off2 = 3, color2 = little_right_pair_b;
                       break;
            case '\\': y = 1, x = 53, off = 3, color = little_right_pair_b;
                       break;
            case '|' : y = 7, x = 1, off = 7, color = little_left_pair_b;
                       y2 = 1, x2 = 53, off2 = 3, color2 = little_right_pair_b;
                       break;
            case '`' : y = 1, x = 57, off = 3, color = little_right_pair_b;
                       break;
            case '~' : y = 7, x = 1, off = 7, color = little_left_pair_b;
                       y2 = 1, x2 = 57, off2 = 3, color2 = little_right_pair_b;
        }
    mvwchgat(keyboardwin, y, x, off, A_NORMAL, color, NULL);
    if(y2)
        mvwchgat(keyboardwin, y2, x2, off2, A_NORMAL, color2, NULL);
    wrefresh(keyboardwin);
    mvwchgat(keyboardwin, y, x, off, A_BOLD, 0, NULL);
    if(y2)
        mvwchgat(keyboardwin, y2, x2, off2, A_BOLD, 0, NULL);
}

void update_wordlist(char *filename){
    int i; 

    free(wordlength);    
    for(i = 0; i < init_words; i++)
        free(wordlist[i]);
    free(wordlist);
    set_wordlist(filename);
}

void set_wordlength(FILE *fp){
    int c, prev_c, count, wlength;

    init_words = 0;
    prev_c = ' ';
    while((c = fgetc(fp)) != EOF){
        if(c == '\n' || c == ' ' || c == '\t'){
            if(prev_c != '\n' && prev_c != ' ' && prev_c != '\t')
                init_words++;
        }
        prev_c = c;
    }
    wordlength = malloc(init_words * sizeof(int));
    rewind(fp);
    for(count = 0; count < init_words; count++){
        wlength = 0;
        prev_c = ' ';
        while((c = fgetc(fp)) != EOF){
            if(c != '\n' && c != ' ' && c != '\t')
                wlength++;
            else if(prev_c != '\n' && prev_c != ' ' && prev_c != '\t')
                break;
            prev_c = c;
        }
        wordlength[count] = wlength;
    }
    rewind(fp);
}

void set_wordlist(char *filename){
    FILE *fp;
    int i, j, c, prev_c;

    fp = fopen(filename, "r");
    set_wordlength(fp);
    wordlist = malloc(init_words * sizeof(char *));
    for(i = 0; i < init_words; i++){
        j = 0;
        wordlist[i] = malloc((wordlength[i] + 1)* sizeof(char));
        prev_c = ' ';
        while((c = fgetc(fp)) != EOF){
            if(c != '\n' && c != ' ' && c != '\t')
                wordlist[i][j++] = c;
            else if(prev_c != '\n' && prev_c != ' ' && prev_c != '\t')
                break;
            prev_c = c;
        }
        wordlist[i][j] = '\0';
    }
    fclose(fp);
}

void sort_wordlists(struct wordlists_t *wordlists){
    int i, j, k, bestval, bestnum, tmpval;
    char *tmp;
    double tmpval2;

    for(i = 0; i < wordlists->num_entries; i++){
        bestnum = i;
        bestval = wordlists->lists[i][0];
        for(j = i + 1; j < wordlists->num_entries; j++){
            if(wordlists->lists[j][0] < bestval){
                bestnum = j;
                bestval = wordlists->lists[j][0];
            }else if(wordlists->lists[j][0] == bestval){
                for(k = 1; k < strlen(wordlists->lists[bestnum]) && 
                        k < strlen(wordlists->lists[j]); k++){
                    if(wordlists->lists[bestnum][k] < wordlists->lists[j][k])
                        break;
                    else if(wordlists->lists[bestnum][k] > wordlists->lists[
                            j][k]){
                        bestnum = j;
                        bestval = wordlists->lists[j][0];
                        break;
                    }
                }
                if(strlen(wordlists->lists[bestnum]) != strlen(wordlists->lists[
                            j])){
                    if(k == strlen(wordlists->lists[j])){
                        bestnum = j;
                        bestval = wordlists->lists[j][0];
                    }
                }
            }
        }
        if(bestnum != i){
            tmpval = wordlists->num_written[i];
            wordlists->num_written[i] = wordlists->num_written[bestnum];
            wordlists->num_written[bestnum] = tmpval;
            tmpval2 = wordlists->best_wpm[i];
            wordlists->best_wpm[i] = wordlists->best_wpm[bestnum];
            wordlists->best_wpm[bestnum] = tmpval2;
            tmpval2 = wordlists->avg_wpm[i];
            wordlists->avg_wpm[i] = wordlists->avg_wpm[bestnum];
            wordlists->avg_wpm[bestnum] = tmpval2;
            tmpval2 = wordlists->avg_acc[i];
            wordlists->avg_acc[i] = wordlists->avg_acc[bestnum];
            wordlists->avg_acc[bestnum] = tmpval2;
            tmp = malloc(sizeof(char) * strlen(wordlists->lists[i]) + 1);
            strcpy(tmp, wordlists->lists[i]);
            wordlists->lists[i] = realloc(wordlists->lists[i],
                    sizeof(char) * strlen(wordlists->lists[bestnum]) + 1);
            strcpy(wordlists->lists[i], wordlists->lists[bestnum]);
            wordlists->lists[bestnum] = realloc(wordlists->lists[bestnum],
                    sizeof(char) * strlen(tmp) + 1);
            strcpy(wordlists->lists[bestnum], tmp);
            free(tmp);
            if(!strcmp(wordlists->lists[i], get_wordlist_name(list)))
                list_index = i;
        }
    }
}

int find_list(FILE *fp, char *list){
    char c, *curlist;
    int i, first; 

    curlist = malloc(MAX_LIST_NAME_LENGTH);
    first = 1;
    while((c = fgetc(fp)) != EOF){
        if(first && c != ' ' && c != '\t' && c != '\n' && c != '#'){
            i = 0;
            memset(curlist, 0, sizeof(*curlist));
            curlist[i] = c;
            while((c = fgetc(fp)) != ' ' && c != '\t' && c != '\n' && 
                    i + 2 < MAX_LIST_NAME_LENGTH)
                curlist[++i] = c;
            curlist[++i] = '\0';
            if(!strcmp(curlist, list)){
                free(curlist);
                return 1;
            }
            first = 0;
        }else{
            if(c == '\n')
                first = 1;
            else
                first = 0;
        }
    }
    free(curlist);
    return 0;
}

void update_wordlist_stats(struct wordlists_t *wordlists, int index,
        double wpm, double acc){
    FILE *stats_fp, *tmp_fp;
    long off;
    int found, cur_num_written;
    double cur_avg_wpm, cur_best_wpm, cur_avg_acc;
    char c;

    stats_fp = fopen(stats_location, "rw");
    tmp_fp = fopen(tmp_location, "w");
    found = find_list(stats_fp, wordlists->lists[index]);
    off = ftell(stats_fp);
    rewind(stats_fp);
    while(ftell(tmp_fp) != off)
        fputc(fgetc(stats_fp), tmp_fp);
    if(found){
        fscanf(stats_fp, "%d %lf %lf %lf", &cur_num_written, &cur_best_wpm, 
                &cur_avg_wpm, &cur_avg_acc);
        cur_num_written++;
        cur_best_wpm = wpm > cur_best_wpm ? wpm : cur_best_wpm;
        cur_avg_wpm = ((double)(cur_num_written - 1) /
                (cur_num_written)) * cur_avg_wpm + ((double)1 / 
                (cur_num_written)) * wpm;
        cur_avg_acc = ((double)(cur_num_written- 1) /
                (cur_num_written)) * cur_avg_acc + ((double)1 /
                (cur_num_written)) * acc;
        fprintf(tmp_fp, "%d %.0lf %lf %lf", cur_num_written, cur_best_wpm, 
                cur_avg_wpm, cur_avg_acc);
        wordlists->num_written[index] = cur_num_written;
        wordlists->best_wpm[index] = cur_best_wpm;
        wordlists->avg_wpm[index] = cur_avg_wpm;
        wordlists->avg_acc[index] = cur_avg_acc;
        while((c = fgetc(stats_fp)) != EOF)
            fputc(c, tmp_fp);
    }else{
        fprintf(tmp_fp, "%s %d %.0lf %lf %lf\n", wordlists->lists[index], 1, wpm,
                wpm, acc);
        wordlists->num_written[index] = 1;
        wordlists->best_wpm[index] = wordlists->avg_wpm[index] = wpm;
        wordlists->avg_acc[index] = acc;
    }
    fclose(stats_fp);
    fclose(tmp_fp);
    remove(stats_location);
    rename(tmp_location, stats_location);
}

void get_wordlist_stats(struct wordlists_t *wordlists, int index){
    FILE *stats_fp;

    stats_fp = fopen(stats_location, "r");
    if(find_list(stats_fp, wordlists->lists[index]))
        fscanf(stats_fp, "%d %lf %lf %lf", &wordlists->num_written[index],
               &wordlists->best_wpm[index], &wordlists->avg_wpm[index],
               &wordlists->avg_acc[index]);
    else{
        wordlists->num_written[index] = 0;
        wordlists->best_wpm[index] = wordlists->avg_wpm[index] = 0;
        wordlists->avg_acc[index] = 100;
    }
    fclose(stats_fp);
}

struct wordlists_t *get_avail_wordlists(char *folder){
    DIR *d;
    struct dirent *dir;
    struct wordlists_t *wordlists;
    int i;

    wordlists = malloc(sizeof(*wordlists));
    wordlists->num_entries = i = 0;
    d = opendir(folder);
    if(d){
        while((dir = readdir(d)) != NULL){
            if(strcmp(dir->d_name, ".") && strcmp(dir->d_name, ".."))
                wordlists->num_entries++;
        }
        rewinddir(d);
        wordlists->lists = malloc(sizeof(char *) * wordlists->num_entries);
        wordlists->num_written = malloc(sizeof(int) * wordlists->num_entries);
        wordlists->best_wpm = malloc(sizeof(double) * wordlists->num_entries);
        wordlists->avg_wpm = malloc(sizeof(double) * wordlists->num_entries);
        wordlists->avg_acc = malloc(sizeof(double) * wordlists->num_entries);
        while((dir = readdir(d)) != NULL){
            if(strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..")){
                wordlists->lists[i] = malloc(sizeof(char) * 
                        strlen(dir->d_name) + 1);
                strcpy(wordlists->lists[i], dir->d_name);
                get_wordlist_stats(wordlists, i++);
            }
        }
    }
    closedir(d);
    sort_wordlists(wordlists);

    return wordlists;
}

struct indices_t* get_rand_wordindices(){
    int i, j, currentInd;
    struct indices_t *indices, *tmpindices; 

    indices = malloc(sizeof(*indices));
    tmpindices = malloc(sizeof(*tmpindices));
    tmpindices->content = malloc(MAX_WORDS * sizeof(int));
    currentInd = 0;
    for(i = 0; i < MAX_WORDS && currentInd - 1 <= PT_XLENGTH; i++){
        tmpindices->content[i] = rand() % init_words;
        currentInd += wordlength[tmpindices->content[i]] + 1;
    }
    i--;
    indices->content = malloc(i * sizeof(int));
    indices->count = i;
    for(j = 0; j < i; j++)
        indices->content[j] = tmpindices->content[j];
    free(tmpindices->content);
    free(tmpindices);
    return indices;
}

struct indices_t* get_iter_wordindices(int word_count){
    int i, j, currentInd;
    struct indices_t *indices, *tmpindices;

    indices = malloc(sizeof(*indices));
    tmpindices = malloc(sizeof(*tmpindices));
    tmpindices->content = malloc(MAX_WORDS * sizeof(int));
    currentInd = 0;
    for(i = 0; i < MAX_WORDS && i + (init_words - word_count) < init_words 
            && currentInd - 1 <= PT_XLENGTH; i++){ 
        tmpindices->content[i] = i + (init_words - word_count);
        currentInd += wordlength[tmpindices->content[i]] + 1;
    }
    i -= currentInd  >= PT_XLENGTH ? 1 : 0;
    indices->content = malloc(i * sizeof(int));
    indices->count = i;
    for(j = 0; j < i; j++)
        indices->content[j] = tmpindices->content[j];
    free(tmpindices->content);
    free(tmpindices);
    return indices;
}

struct indices_t* get_consume_rand_wordindices(struct wordinfo_t *info){
    int i, j, currentInd, randInd, tmp;
    struct indices_t *indices, *tmpindices;

    indices = malloc(sizeof(*indices));
    tmpindices = malloc(sizeof(*tmpindices));
    tmpindices->content = malloc(MAX_WORDS * sizeof(int));
    currentInd = 0;
    for(i = 0; i < MAX_WORDS && init_words - info->length < init_words 
            && currentInd - 1 <= PT_XLENGTH; i++){ 
        randInd = rand() % info->length;
        tmpindices->content[i] = info->indices[randInd];
        tmp = info->indices[randInd];
        info->indices[randInd] = info->indices[info->length-1];
        info->indices[info->length-1] = tmp;
        info->length--;
        currentInd += wordlength[tmpindices->content[i]] + 1;
    }
    if(currentInd >= PT_XLENGTH){
        info->length++;
        i--;
    }
    indices->content = malloc(i * sizeof(int));
    indices->count = i;
    for(j = 0; j < i; j++)
        indices->content[j] = tmpindices->content[j];
    free(tmpindices->content);
    free(tmpindices);
    return indices;
}

void print_words(struct indices_t *indices, int row){
    int i;

    wmove(printwin, row, 0);
    for(i = 0; i < indices->count; i++){
        waddstr(printwin, wordlist[indices->content[i]]);
        if(i != indices->count - 1)
            waddch(printwin, ' ');
    }
    wrefresh(printwin);
}

char *get_wordlist_name(char *list){
    int i;
    char *listname;

    listname = &list[0];
    for(i = 0; i < strlen(list); i++){
        if(list[i] == '/')
            listname = &list[i + 1];
    }
    return listname;
}

void refresh_screen(){
    redrawwin(stdscr);
    redrawwin(titlewin);
    redrawwin(typewin);
    redrawwin(printwin);
    redrawwin(clockwordwin);
    redrawwin(optionwin);
    redrawwin(infowin);
    redrawwin(progresswin);
    refresh();
    wrefresh(titlewin);
    wrefresh(typewin);
    wrefresh(printwin);
    wrefresh(clockwordwin);
    wrefresh(optionwin);
    wrefresh(infowin);
    wrefresh(progresswin);
    move(type_y + 7, type_x + PT_XSTART);
    refresh();
}

void update_progress_step(){
    if(timed())
        progress_step = (double)init_secs / max_x;
    else
        progress_step = (double)init_words / max_x;
}

void write_to_file(char *filename){
    FILE *oldfp, *newfp;
    int list_found, mode_found, advance_found, time_found, first, i;
    char c, *param;

    param = malloc(MAX_PARAM_SIZE * sizeof(char));
    list_found = mode_found = advance_found = time_found = 0;
    first = 1;
    oldfp = fopen(filename, "r");
    newfp = fopen(tmp_location, "w");
    if(oldfp != NULL){
        while((c = fgetc(oldfp)) != EOF){
            fputc(c , newfp);
            if(first && (c == ' ' || c == '#'))
                first = 0;
            else if(c == '\n')
                first = 1;
            else if(first){
                first = 0;
                for(i = 0; i < MAX_PARAM_SIZE; i++)
                    param[i] = '\0';
                param[0] = c;
                fscanf(oldfp, "%[^=]", &param[1]);
                fprintf(newfp, "%s", &param[1]);
                if(strcmp(param, "wordlist") == 0){
                    list_found = 1;
                    fgetc(oldfp);
                    fputc('=', newfp);
                    fprintf(newfp, "%s", list);
                    while((c = fgetc(oldfp)) != '\n' && c != EOF)
                        ;
                    if(c == '\n'){
                        fputc('\n', newfp);
                    }else
                        break;
                    first = 1;
                }else if(strcmp(param, "mode") == 0){
                    mode_found = 1;
                    fgetc(oldfp);
                    fputc('=', newfp);
                    fprintf(newfp, "%s", mode == 0 ? "consume" : "timed");
                    while((c = fgetc(oldfp)) != '\n' && c != EOF)
                        ;
                    if(c == '\n'){
                        fputc('\n', newfp);
                    }else
                        break;
                    first = 1;
                }else if(strcmp(param, "advance-mode") == 0){
                    advance_found = 1;
                    fgetc(oldfp);
                    fputc('=', newfp);
                    fprintf(newfp, "%s", iterative == 0 ? "random" :
                            "iterative");
                    while((c = fgetc(oldfp)) != '\n' && c != EOF)
                        ;
                    if(c == '\n'){
                        fputc('\n', newfp);
                    }else
                        break;
                    first = 1;
                }else if(strcmp(param, "time") == 0){
                    time_found = 1;
                    fgetc(oldfp);
                    fputc('=', newfp);
                    fprintf(newfp, "%d", init_secs);
                    while((c = fgetc(oldfp)) != '\n' && c != EOF)
                        ;
                    if(c == '\n'){
                        fputc('\n', newfp);
                    }else
                        break;
                    first = 1;
                }
            }
        }
    }
    if(!list_found){
        if(!first) 
            fputc('\n', newfp);
        fprintf(newfp, "wordlist=%s", list);
    }
    if(!mode_found){
        fputc('\n', newfp);
        fprintf(newfp, "mode=%s", mode == 0 ? "consume" : "timed");
    }
    if(!advance_found){
        fputc('\n', newfp);
        fprintf(newfp, "advance-mode=%s", iterative == 0 ? "random" :
                "iterative");
    }
    if(!time_found){
        fputc('\n', newfp);
        fprintf(newfp, "time=%d", init_secs);
    }
    free(param);
    fclose(newfp);
    if(oldfp != NULL){
        fclose(oldfp);
        remove(filename);
    }
    rename(tmp_location, filename);
}

struct indices_t *search_for_list(struct wordlists_t *wordlists, char *str){
    int i, j, bestfit, numfit, tmpfit, curlen;
    struct indices_t *indices;

    curlen = 5;
    indices = malloc(sizeof(*indices));
    indices->content = malloc(sizeof(int) * curlen);
    indices->count = 0;
    bestfit = 0;
    for(i = 0; i < wordlists->num_entries; i++){
        numfit = tmpfit = 0;
        for(j = 0; j <= strlen(wordlists->lists[i]); j++){
            if(tmpfit == strlen(str)){
                numfit = tmpfit;
                break;
            }else if(str[tmpfit] == wordlists->lists[i][j])
                tmpfit++;
            else{
                if(tmpfit > numfit)
                    numfit = tmpfit;
                tmpfit = 0;
            }
        }
        if(numfit == bestfit){
            if(indices->count  == curlen){
                curlen *= 2;
                indices->content = realloc(indices->content, sizeof(int) *
                        curlen);
            }
            indices->content[indices->count] = i;
            indices->count++;
        }else if(numfit > bestfit){
            curlen = 5;
            indices->content = realloc(indices->content, sizeof(int) * curlen);
            bestfit = numfit;
            indices->count = 1;
            indices->content[0] = i;
        }
    }
    if(bestfit < strlen(str))
        indices->count = 0;

    return indices;
}

int get_len(int val){
    int len;

    if(val == 0)
        return 1;
    len = 0;
    while(val != 0){
        len++;
        val /= 10;
    }
    
    return len;
}

void change_params(){
    WINDOW *paramwin, *wordlistwin;
    int i, j, off, count, len, search_len, tmp_ind, cur_elem, 
        indices_initialized, modified, left, print_len;
    char c, search_c, t[MAX_TIME_LENGTH], search_s[MAX_LIST_NAME_LENGTH]; 
    struct indices_t *indices;
    struct timeval timeout;

    timeout.tv_sec = 0;
    timeout.tv_usec = 10;
    indices_initialized = off = 0; 
    paramwin = newwin(max_y - 3, max_x, 3, 0);
    wordlistwin = newwin(WL_YLENGTH, max_x - 17, WL_YSTART,
            WL_XSTART); 
    scrollok(wordlistwin, true);
    len =  modified = 0;

    left = max_x - MAX_LIST_NAME_LENGTH - 26;
    mvaddch(2, max_x - 16, ACS_TTEE);
    werase(optionwin);
    wattr_set(optionwin, A_BOLD, paramwin_pair_f, NULL);
    mvwprintw(optionwin, 0, 0, "[PARAMWINDOW]");
    mvwprintw(optionwin, 0, max_x - 15, "[CONTINUE: 'c']");
    mvwhline(paramwin, 1, 0, 0, max_x - 16); 
    mvwhline(paramwin, WL_YLENGTH + 2, 0, 0, max_x); 
    mvwvline(paramwin, 0, max_x - 16, 0, WL_YLENGTH + 2);
    mvwaddch(paramwin, 1, max_x - 16, ACS_RTEE);
    mvwaddch(paramwin, WL_YLENGTH + 2, max_x - 16, ACS_BTEE);
    mvwprintw(paramwin, 0, MAX_LIST_NAME_LENGTH + 2, "#Written");
    mvwprintw(paramwin, 0, MAX_LIST_NAME_LENGTH + 2 + left / 3.0, "Best Wpm");
    mvwprintw(paramwin, 0, MAX_LIST_NAME_LENGTH + 2 + left * (2/3.0),
            "Avg Wpm");
    mvwprintw(paramwin, 0, MAX_LIST_NAME_LENGTH + 2 + left, "Avg Acc");
    mvwprintw(paramwin, 0, 0, "Wordlists");
    mvwprintw(paramwin, WL_YLENGTH + 3, 0, "Current Wordlist: %s", 
            get_wordlist_name(list));
    mvwprintw(paramwin, 0, max_x - 15, "Mode");
    mvwprintw(paramwin, 1, max_x - 15, "%s", mode ? "timed" : "consume");
    mvwprintw(paramwin, (max_y - 8) / 2, max_x - 15, "Advance-Mode");
    mvwprintw(paramwin, (max_y - 8) / 2 + 1, max_x - 15, "%s", iterative ? 
            "iterative" : "random");
    mvwprintw(paramwin, max_y - 8, max_x - 15, "Time");
    mvwprintw(paramwin, max_y - 7, max_x - 15, "%d", init_secs);

    mvwchgat(paramwin, 0, 0, 9, A_BOLD, 0, NULL);
    mvwchgat(paramwin, WL_YLENGTH + 3, 0, 16, A_BOLD, 0, NULL);
    mvwchgat(paramwin, 0, MAX_LIST_NAME_LENGTH + 2, 8, A_BOLD,
            num_written_pair_f, NULL);
    mvwchgat(paramwin, 0, MAX_LIST_NAME_LENGTH + 2 + left / 3.0, 8, A_BOLD,
            best_wpm_pair_f, NULL);
    mvwchgat(paramwin, 0, MAX_LIST_NAME_LENGTH + 2 + left * (2/3.0), 7, A_BOLD,
            avg_wpm_pair_f, NULL);
    mvwchgat(paramwin, 0, MAX_LIST_NAME_LENGTH + 2 + left, 7, A_BOLD,
            avg_acc_pair_f, NULL);
    mvwchgat(paramwin, 0, max_x - 15, 4, A_BOLD, 0, NULL);
    mvwchgat(paramwin, (max_y - 8) / 2, max_x - 15, 12, A_BOLD, 0, NULL);
    mvwchgat(paramwin, max_y - 8, max_x - 15, 4, A_BOLD, 0, NULL);

    for(i = 0; i < wordlists->num_entries && i < WL_YLENGTH; i++){
        mvwprintw(wordlistwin, i, 0, "%s", wordlists->lists[i]);
        mvwprintw(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2, "%d", 
                wordlists->num_written[i]);
        mvwchgat(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2, 
                get_len(wordlists->num_written[i]), A_NORMAL, num_written_pair_f
                , NULL);
        mvwprintw(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2 + left / 3.0, "%.0lf"
                , wordlists->best_wpm[i]);
        mvwchgat(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2 + left / 3.0, 
                get_len(wordlists->best_wpm[i]), A_NORMAL, best_wpm_pair_f,
                NULL);
        mvwprintw(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2 + left * (2/3.0),
                "%.0lf", wordlists->avg_wpm[i]);
        mvwchgat(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2 + left * (2/3.0),
                get_len((int)wordlists->avg_wpm[i] + 1),
                A_NORMAL, avg_wpm_pair_f, NULL);
        mvwprintw(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2 + left, "%.1lf", 
                wordlists->avg_acc[i]);
        waddch(wordlistwin, '%');
        mvwchgat(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2 + left,
                get_len((int)wordlists->avg_acc[i] + 1) + 3, A_NORMAL, 
                avg_acc_pair_f, NULL);
    }
    mvwchgat(wordlistwin, 0, 0, strlen(wordlists->lists[0]),A_REVERSE, 0, NULL);
    mvwchgat(wordlistwin, 0, MAX_LIST_NAME_LENGTH + 2, 
            get_len(wordlists->num_written[0]), A_BOLD, num_written_pair_f,
            NULL);
    mvwchgat(wordlistwin, 0, MAX_LIST_NAME_LENGTH + 2 + left / 3.0,
            get_len(wordlists->best_wpm[0]), A_BOLD, best_wpm_pair_f, NULL);
    mvwchgat(wordlistwin, 0, MAX_LIST_NAME_LENGTH + 2 + left * (2/3.0),
            get_len((int)wordlists->avg_wpm[0] + 1), A_BOLD, avg_wpm_pair_f,
            NULL);
    mvwchgat(wordlistwin, 0, MAX_LIST_NAME_LENGTH + 2 + left,
            get_len((int)wordlists->avg_acc[0] + 1) + 3, A_BOLD, avg_acc_pair_f,
            NULL);
    move(WL_YSTART, WL_XSTART - 1);
    wrefresh(paramwin);
    wrefresh(wordlistwin);
    wrefresh(optionwin);
    count = i;

    while((c = getch()) != 'c'){
        if(c == '\n'){
            modified++;
            wmove(paramwin, WL_YLENGTH + 3, 18);
            for(j = 0; j < strlen(get_wordlist_name(list)); j++)
                waddch(paramwin, ' ');
            free(list);
            list = malloc(strlen(list_location) + 
                    strlen(wordlists->lists[i - count + off]) + 1);
            strcpy(list, list_location);
            strcat(list, wordlists->lists[i - count + off]);
            list_index = i - count + off;
            update_wordlist(list);
            update_progress_step();
            mvwprintw(paramwin, WL_YLENGTH + 3, 18, "%s",
                    get_wordlist_name(list));
            wrefresh(paramwin);
            len = strlen("<| 10type : ") + strlen(get_wordlist_name(list)) +
                strlen(" |>");
            wresize(titlewin, 1, len);
            mvwin(titlewin, 0, max_x / 2 - len / 2);
        }else if(c == 'm'){
            modified++;
            mode = mode == 0 ? 1 : 0;
            update_progress_step();
            wmove(paramwin, 1, max_x - 15);
            wclrtoeol(paramwin);
            mvwprintw(paramwin, 1, max_x - 15, "%s", mode ? "timed" :
                    "consume");
        }else if(c == 't'){
            modified++;
            for(j = 0; j < MAX_TIME_LENGTH; j++)
                t[j] = '\0';
            j = 0;
            wmove(paramwin, max_y - 7, max_x - 15);
            wclrtoeol(paramwin);
            wmove(paramwin, max_y - 7, max_x - 15);
            wrefresh(paramwin);
            while((c = getch()) != '\n' && j < MAX_TIME_LENGTH){
                if(c > 47 && c < 58){
                    if(c == 48 && j == 0)
                        ;
                    else{
                        waddch(paramwin, c);
                        t[j++] = c;
                        wrefresh(paramwin);
                    }
                }
            }
            init_secs = atoi(t);
            if(!init_secs){
                init_secs = 1;
                mvwaddch(paramwin, max_y - 7, max_x - 15, '1');
            }
            update_progress_step();
        }else if(c == 'a'){
            modified++;
            iterative = iterative == 0 ? 1 : 0;
            wmove(paramwin, (max_y - 8) / 2 + 1, max_x - 15);
            wclrtoeol(paramwin);
            mvwprintw(paramwin, (max_y - 8) / 2 + 1, max_x - 15, "%s",
                    iterative ? "iterative" : "random");
        }else if(c == 'w')
            write_to_file(conf_location);
        else{
            if(c == ESCAPE){
                if(select(1, NULL, (fd_set *)stdin, NULL, &timeout) == 0)
                    continue;
                c = getch();
            }
            if(c == 'j' || c == 'B'){
                if(i == wordlists->num_entries && off + 1 < count){
                    mvwchgat(wordlistwin, off, 0, strlen(wordlists->lists[(i
                                    -count) +  off]), A_NORMAL, 0, NULL);
                    mvwchgat(wordlistwin, off, 
                            MAX_LIST_NAME_LENGTH + 2,
                            get_len(wordlists->num_written[(i - count) + off]),
                            A_NORMAL, num_written_pair_f, NULL);
                    mvwchgat(wordlistwin, off, 
                            MAX_LIST_NAME_LENGTH + 2 + left / 3.0,
                            get_len(wordlists->best_wpm[(i - count) + off]),
                            A_NORMAL, best_wpm_pair_f, NULL);
                    mvwchgat(wordlistwin, off,
                            MAX_LIST_NAME_LENGTH + 2 + left * (2/3.0),
                            get_len((int)wordlists->avg_wpm[(i - count) + off]
                            +1), A_NORMAL, avg_wpm_pair_f, NULL);
                    mvwchgat(wordlistwin, off,
                            MAX_LIST_NAME_LENGTH + 2 + left,
                            get_len((int)wordlists->avg_acc[(i - count) +
                            off++] + 1) + 3,A_NORMAL, avg_acc_pair_f, NULL);
                }else if(i < wordlists->num_entries){
                    scroll(wordlistwin);
                    mvwprintw(wordlistwin, WL_YLENGTH - 1, 0, "%s",
                            wordlists->lists[i]);
                    mvwprintw(wordlistwin, WL_YLENGTH - 1, MAX_LIST_NAME_LENGTH
                            + 2, "%d", wordlists->num_written[i]);
                    mvwprintw(wordlistwin, WL_YLENGTH - 1, MAX_LIST_NAME_LENGTH
                            + 2 + left / 3.0, "%.0lf", wordlists->best_wpm[i]);
                    mvwprintw(wordlistwin, WL_YLENGTH - 1, MAX_LIST_NAME_LENGTH
                            + 2 + left * (2/3.0), "%.0lf", 
                            wordlists->avg_wpm[i]);
                    mvwprintw(wordlistwin, WL_YLENGTH - 1, MAX_LIST_NAME_LENGTH
                            + 2 + left, "%.1lf", wordlists->avg_acc[i]);
                    waddch(wordlistwin, '%');
                    mvwchgat(wordlistwin, WL_YLENGTH - 1, MAX_LIST_NAME_LENGTH
                            + 2, get_len(wordlists->num_written[i]), A_NORMAL,
                            num_written_pair_f, NULL);
                    mvwchgat(wordlistwin, WL_YLENGTH - 1, MAX_LIST_NAME_LENGTH
                            + 2 + left / 3.0, get_len(wordlists->best_wpm[i]),
                            A_NORMAL, best_wpm_pair_f, NULL);
                    mvwchgat(wordlistwin, WL_YLENGTH - 1, MAX_LIST_NAME_LENGTH
                            + 2 + left * (2/3.0), get_len(
                            (int)wordlists->avg_wpm[i] + 1) , A_NORMAL,
                            avg_wpm_pair_f, NULL);
                    mvwchgat(wordlistwin, WL_YLENGTH - 1, MAX_LIST_NAME_LENGTH
                            + 2 + left, get_len((int)wordlists->avg_acc[i++] + 1
                            ) + 3, A_NORMAL, avg_acc_pair_f, NULL);
                }
                if(indices_initialized && indices->count > 1)
                    if(cur_elem <= indices->count - 2 && i - count + off >=
                            indices->content[cur_elem + 1])
                        cur_elem++;
            }else if(c == 'k' || c == 'A'){
                if(off){
                    mvwchgat(wordlistwin, off, 0, strlen(wordlists->lists[(i
                                    - count) + off]), A_NORMAL, 0, NULL);
                    mvwchgat(wordlistwin, off, MAX_LIST_NAME_LENGTH + 2,
                            get_len(wordlists->num_written[(i - count) + off]),
                            A_NORMAL, num_written_pair_f, NULL);
                    mvwchgat(wordlistwin, off, MAX_LIST_NAME_LENGTH + 2 +
                            left / 3.0, get_len(wordlists->best_wpm[(i - count)
                            + off]),A_NORMAL, best_wpm_pair_f, NULL);
                    mvwchgat(wordlistwin, off, MAX_LIST_NAME_LENGTH + 2 +
                            left * (2/3.0), get_len((int)wordlists->avg_wpm[(i - 
                            count) + off] + 1), A_NORMAL, avg_wpm_pair_f, NULL);
                    mvwchgat(wordlistwin, off, MAX_LIST_NAME_LENGTH + 2 + left,
                            get_len((int)wordlists->avg_acc[(i - count) + off--
                            ] + 1) + 3, A_NORMAL, avg_acc_pair_f, NULL);
                }else if(i - count > 0){
                    mvwchgat(wordlistwin, 0, 0, strlen(wordlists->lists[i -
                                count]), A_NORMAL, 0, NULL);
                    mvwchgat(wordlistwin, 0, 
                            MAX_LIST_NAME_LENGTH + 2, 
                            get_len(wordlists->num_written[i - count]),
                            A_NORMAL, num_written_pair_f, NULL);
                    mvwchgat(wordlistwin, 0, 
                            MAX_LIST_NAME_LENGTH + 2 + left / 3.0,
                            get_len(wordlists->best_wpm[i - count]), A_NORMAL,
                            best_wpm_pair_f, NULL);
                    mvwchgat(wordlistwin, 0,
                            MAX_LIST_NAME_LENGTH + 2 + left * (2/3.0),
                            get_len((int)wordlists->avg_wpm[i - count] + 1), 
                            A_NORMAL, avg_wpm_pair_f, NULL);
                    mvwchgat(wordlistwin, 0,
                            MAX_LIST_NAME_LENGTH + 2 + left,
                            get_len((int)wordlists->avg_acc[i - count] + 1) + 3,
                            A_NORMAL, avg_acc_pair_f, NULL);
                    wscrl(wordlistwin, -1);
                    mvwprintw(wordlistwin, 0, 0, "%s", wordlists->lists[--i - 
                            count]);
                    mvwprintw(wordlistwin, 0, MAX_LIST_NAME_LENGTH + 2, "%d",
                            wordlists->num_written[i - count]);
                    mvwprintw(wordlistwin, 0, MAX_LIST_NAME_LENGTH + 2 + left /
                            3.0, "%.0lf", wordlists->best_wpm[i - count]);
                    mvwprintw(wordlistwin, 0, MAX_LIST_NAME_LENGTH + 2 + left *
                            (2/3.0), "%.0lf", wordlists->avg_wpm[i - count]);
                    mvwprintw(wordlistwin, 0, MAX_LIST_NAME_LENGTH + 2 + left, 
                            "%.1lf", wordlists->avg_acc[i - count]);
                    waddch(wordlistwin, '%');
                    mvwchgat(wordlistwin, 0, MAX_LIST_NAME_LENGTH + 2, 
                            get_len(wordlists->num_written[i - count]),
                            A_NORMAL, num_written_pair_f, NULL);
                    mvwchgat(wordlistwin, 0, MAX_LIST_NAME_LENGTH + 2 + left /
                            3.0, get_len(wordlists->best_wpm[i - count]),
                            A_NORMAL, best_wpm_pair_f, NULL);
                    mvwchgat(wordlistwin, 0, MAX_LIST_NAME_LENGTH + 2 + left *
                            (2/3.0), get_len((int)wordlists->avg_wpm[i - count]
                            + 1), A_NORMAL, avg_wpm_pair_f, NULL);
                    mvwchgat(wordlistwin, 0, MAX_LIST_NAME_LENGTH + 2 + left,
                            get_len((int)wordlists->avg_acc[i - count] + 1) + 3,
                            A_NORMAL, avg_acc_pair_f, NULL);
                }
                if(indices_initialized && indices->count > 1){
                    if(cur_elem >= 1 && i - count + off <= indices->content[
                            cur_elem - 1])
                        cur_elem--;
                }
            }else if(c == 'g'){
                werase(wordlistwin);
                if(indices_initialized && indices->count > 1)
                    cur_elem = indices->content[0] == 0 ? 0 : -1;
                for(i = 0; i < wordlists->num_entries && i < WL_YLENGTH; i++){
                    mvwprintw(wordlistwin, i, 0, "%s", wordlists->lists[i]);
                    mvwprintw(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2, "%d",
                            wordlists->num_written[i]);
                    mvwprintw(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2 + left /
                            3.0, "%.0lf", wordlists->best_wpm[i]);
                    mvwprintw(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2 + left *
                            (2/3.0), "%.0lf", wordlists->avg_wpm[i]);
                    mvwprintw(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2 + left,
                            "%.1lf", wordlists->avg_acc[i]);
                    waddch(wordlistwin, '%');
                    mvwchgat(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2, 
                            get_len(wordlists->num_written[i]), A_NORMAL,
                            num_written_pair_f, NULL); 
                    mvwchgat(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2 + left /
                            3.0, get_len(wordlists->best_wpm[i]), A_NORMAL,
                            best_wpm_pair_f, NULL); 
                    mvwchgat(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2 + left *
                            (2/3.0), get_len((int)wordlists->avg_wpm[i] + 1),
                            A_NORMAL, avg_wpm_pair_f, NULL); 
                    mvwchgat(wordlistwin, i, MAX_LIST_NAME_LENGTH + 2 + left, 
                            get_len((int)wordlists->avg_acc[i] + 1) + 3,
                            A_NORMAL, avg_acc_pair_f, NULL); 
                }
                off = 0;
            }else if(c == 'G'){
                werase(wordlistwin);
                if(indices_initialized && indices->count > 1)
                    cur_elem = indices->content[indices->count - 1] == 
                        wordlists->num_entries - 1 ? indices->count - 1 : 
                        indices->count;
                for(i = wordlists->num_entries - count, off = 0; i < 
                        wordlists->num_entries; i++){
                    mvwprintw(wordlistwin, off, 0, "%s", wordlists->lists[i]);
                    mvwprintw(wordlistwin, off, MAX_LIST_NAME_LENGTH + 2, "%d",
                            wordlists->num_written[i]);
                    mvwprintw(wordlistwin, off, MAX_LIST_NAME_LENGTH + 2 + left
                            / 3.0, "%.0lf", wordlists->best_wpm[i]);
                    mvwprintw(wordlistwin, off, MAX_LIST_NAME_LENGTH + 2 + left
                            * (2/3.0), "%.0lf", wordlists->avg_wpm[i]);
                    mvwprintw(wordlistwin, off, MAX_LIST_NAME_LENGTH + 2 + left, 
                            "%.1lf", wordlists->avg_acc[i]);
                    waddch(wordlistwin, '%');
                    mvwchgat(wordlistwin, off, MAX_LIST_NAME_LENGTH + 2, 
                            get_len(wordlists->num_written[i]), A_NORMAL,
                            num_written_pair_f, NULL);
                    mvwchgat(wordlistwin, off, MAX_LIST_NAME_LENGTH + 2 + left /
                            3.0, get_len(wordlists->best_wpm[i]), A_NORMAL,
                            best_wpm_pair_f, NULL);
                    mvwchgat(wordlistwin, off, MAX_LIST_NAME_LENGTH + 2 + left *
                            (2/3.0), get_len((int)wordlists->avg_wpm[i] + 1),
                            A_NORMAL, avg_wpm_pair_f, NULL);
                    mvwchgat(wordlistwin, off++, MAX_LIST_NAME_LENGTH + 2 + left
                            , get_len((int)wordlists->avg_acc[i] + 1) + 3,
                            A_NORMAL, avg_acc_pair_f, NULL);
                }
                off--;
            }else if(c == '/'){
                wmove(paramwin, max_y - 5, 23 + strlen(get_wordlist_name(list)))
                    ;
                wclrtoeol(paramwin);
                mvwaddch(paramwin, max_y - 5, 23 + strlen(get_wordlist_name(
                                list)), ACS_VLINE);
                mvwaddch(paramwin, max_y - 6, 23 + strlen(get_wordlist_name(
                                list)), ACS_TTEE);
                mvwprintw(paramwin, max_y - 5, 25 + strlen(get_wordlist_name(
                                list)), "Search: ");
                move(WL_YSTART + off, 5);
                wrefresh(paramwin);
                for(search_c = getch(), search_len = 0; search_c != '\n' && 
                        search_c != ESCAPE;){
                    if(search_c == 127){
                        if(search_len >= 1){
                            mvwaddch(paramwin, max_y - 5, 32 + strlen(
                                        get_wordlist_name(list)) + search_len,
                                    ' ');
                            wmove(paramwin, max_y - 5, 32 + strlen(
                                        get_wordlist_name(list)) + search_len);
                            search_s[--search_len] = '\0';
                            wrefresh(paramwin);
                        }
                    }else{
                        if(search_len + 2 < MAX_LIST_NAME_LENGTH){
                            waddch(paramwin, search_c);
                            wrefresh(paramwin);
                            search_s[search_len] = search_c;
                            search_s[++search_len] = '\0';
                        }
                    }
                    if(search_len >= 1){
                        if(indices_initialized){
                            free(indices->content);
                            free(indices);
                        }else
                            indices_initialized = 1;
                        indices = search_for_list(wordlists, search_s);
                        if(indices->count > 0){
                            wclrtoeol(paramwin);
                            mvwprintw(paramwin, max_y - 5, 34 + search_len + 
                                    strlen(get_wordlist_name(list)),
                                    indices->count > 1 ? "[%d matches]" :
                                    "[%d match]", indices->count);
                            mvwchgat(paramwin, max_y - 5, 35 + search_len +
                                    strlen(get_wordlist_name(list)),
                                    get_len(indices->count) + (indices->count >
                                    1 ? 8 : 6), A_NORMAL, green_f, NULL);
                            mvwchgat(paramwin, max_y - 5, 33 + strlen(
                                    get_wordlist_name(list)), search_len,
                                    A_NORMAL, 0, NULL);
                            wmove(paramwin, max_y - 5,  33 + strlen(
                                    get_wordlist_name(list)) + search_len);
                            wrefresh(paramwin);
                            cur_elem = 0;
                            i = indices->content[0] + count;
                            werase(wordlistwin);
                            if(i > wordlists->num_entries){
                                off = i - wordlists->num_entries;
                                i = wordlists->num_entries;
                            }else
                                off = 0;
                            for(tmp_ind = i - count; tmp_ind < i; tmp_ind++){
                                mvwprintw(wordlistwin, tmp_ind - (i - count)
                                        , 0, "%s", wordlists->lists[tmp_ind]);
                                mvwprintw(wordlistwin, tmp_ind - (i - count)
                                        , MAX_LIST_NAME_LENGTH + 2, "%d", 
                                        wordlists->num_written[tmp_ind]);
                                mvwprintw(wordlistwin, tmp_ind - (i - count)
                                        , MAX_LIST_NAME_LENGTH + 2 + left / 3.0,
                                        "%.0lf", wordlists->best_wpm[tmp_ind]);
                                mvwprintw(wordlistwin, tmp_ind - (i - count), 
                                        MAX_LIST_NAME_LENGTH + 2 + left * 
                                        (2/3.0), "%.0lf", wordlists->avg_wpm[
                                        tmp_ind]);
                                mvwprintw(wordlistwin, tmp_ind - (i - count)
                                        , MAX_LIST_NAME_LENGTH + 2 + left,
                                        "%.1lf", wordlists->avg_acc[tmp_ind]);
                                waddch(wordlistwin, '%');
                                mvwchgat(wordlistwin, tmp_ind - (i - count),
                                        MAX_LIST_NAME_LENGTH + 2, 
                                        get_len(wordlists->num_written[tmp_ind])
                                        , A_NORMAL, num_written_pair_f, NULL);
                                mvwchgat(wordlistwin, tmp_ind - (i - count),
                                        MAX_LIST_NAME_LENGTH + 2 + left / 3.0,
                                        get_len(wordlists->best_wpm[tmp_ind]),
                                        A_NORMAL, best_wpm_pair_f, NULL);
                                mvwchgat(wordlistwin, tmp_ind - (i - count),
                                        MAX_LIST_NAME_LENGTH + 2 + left * (2/3.0
                                        ), get_len((int)wordlists->avg_wpm[
                                        tmp_ind] + 1), A_NORMAL, avg_wpm_pair_f,
                                        NULL);
                                mvwchgat(wordlistwin, tmp_ind - (i - count),
                                        MAX_LIST_NAME_LENGTH + 2 + left, 
                                        get_len((int)wordlists->avg_acc[tmp_ind]
                                        + 1) + 3, A_NORMAL, avg_acc_pair_f,
                                        NULL);
                            }
                            mvwchgat(wordlistwin, off, 0, strlen(
                                    wordlists->lists[i - count + off]) ,
                                    A_REVERSE, 0, NULL);
                            mvwchgat(wordlistwin, off, 
                                    MAX_LIST_NAME_LENGTH + 2, 
                                    get_len(wordlists->num_written[i - count +
                                    off]), A_BOLD, num_written_pair_f, NULL);
                            mvwchgat(wordlistwin, off, 
                                    MAX_LIST_NAME_LENGTH + 2 + left / 3.0, 
                                    get_len(wordlists->best_wpm[i - count + off]
                                    ), A_BOLD, best_wpm_pair_f, NULL);
                            mvwchgat(wordlistwin, off,
                                    MAX_LIST_NAME_LENGTH + 2 + left * (2/3.0),
                                    get_len((int)wordlists->avg_wpm[i - count +
                                    off] + 1), A_BOLD, avg_wpm_pair_f, NULL);
                            mvwchgat(wordlistwin, off,
                                    MAX_LIST_NAME_LENGTH + 2 + left,
                                    get_len((int)wordlists->avg_acc[i - count +
                                    off] + 1) + 3, A_BOLD, avg_acc_pair_f,
                                    NULL);
                            wrefresh(wordlistwin);
                        }else{
                            wclrtoeol(paramwin);
                            mvwprintw(paramwin, max_y - 5, 34 + search_len +
                                    strlen(get_wordlist_name(list)),
                                    "[no matches]");
                            mvwchgat(paramwin, max_y - 5, 33 + strlen(
                                    get_wordlist_name(list)), search_len,
                                    A_NORMAL, red_b, NULL);
                            mvwchgat(paramwin, max_y - 5, 35 + strlen(
                                    get_wordlist_name(list)) + search_len,
                                    10, A_NORMAL, red_f, NULL);
                            wmove(paramwin, max_y - 5, 33 + strlen(
                                    get_wordlist_name(list)) + search_len);
                            wrefresh(paramwin);
                        }
                    }
                    move(WL_YSTART + off, 5);
                    search_c = getch();
                }
                if(indices_initialized && search_c == ESCAPE)
                    indices->count = 0;
                wmove(paramwin, max_y - 5, 23 + strlen(get_wordlist_name(
                                list)));
                wclrtoeol(paramwin);
                if(indices_initialized && indices->count > 0){
                    print_len = get_len(indices->count) * 2 + 3;
                    mvwprintw(paramwin, max_y - 5, max_x - print_len, "[%*d|%d]"
                            , get_len(indices->count), 1, indices->count);
                }
                mvwaddch(paramwin, max_y - 6, 23 + strlen(get_wordlist_name(
                                    list)), ACS_HLINE);
            }else if((c == 'n' || c == 'N') && indices_initialized &&
                    indices->count != 0){
                werase(wordlistwin);
                if(c == 'n'){
                    if(cur_elem < 0)
                        cur_elem = 0;
                    else
                        cur_elem = indices->content[cur_elem] > i - count + off
                        ? cur_elem : cur_elem >= indices->count-1 ? 0 : cur_elem
                        + 1;
                }else{
                    if(cur_elem > indices->count - 1)
                        cur_elem = indices->count - 1;
                    else
                        cur_elem = indices->content[cur_elem] < i - count + off
                        ? cur_elem : cur_elem <= 0 ? indices->count-1 : cur_elem
                        - 1;
                }
                mvwprintw(paramwin, max_y - 5, max_x - print_len + 1, "%*d",
                        get_len(indices->count), cur_elem + 1);
                i = indices->content[cur_elem] + count;
                if(i > wordlists->num_entries){
                    off = i - wordlists->num_entries;
                    i = wordlists->num_entries;
                }else
                    off = 0;
                for(tmp_ind = i - count; tmp_ind < i; tmp_ind++){
                    mvwprintw(wordlistwin, tmp_ind - (i - count) , 0, "%s",
                            wordlists->lists[tmp_ind]);
                    mvwprintw(wordlistwin, tmp_ind - (i - count) , 
                            MAX_LIST_NAME_LENGTH + 2, "%d",
                            wordlists->num_written[tmp_ind]);
                    mvwprintw(wordlistwin, tmp_ind - (i - count) , 
                            MAX_LIST_NAME_LENGTH + 2 + left / 3.0, "%.0lf",
                            wordlists->best_wpm[tmp_ind]);
                    mvwprintw(wordlistwin, tmp_ind - (i - count) , 
                            MAX_LIST_NAME_LENGTH + 2 + left * (2/3.0), "%.0lf",
                            wordlists->avg_wpm[tmp_ind]);
                    mvwprintw(wordlistwin, tmp_ind - (i - count) , 
                            MAX_LIST_NAME_LENGTH + 2 + left, "%.1lf",
                            wordlists->avg_acc[tmp_ind]);
                    waddch(wordlistwin, '%');
                    mvwchgat(wordlistwin, tmp_ind - (i - count), 
                            MAX_LIST_NAME_LENGTH + 2, 
                            get_len(wordlists->num_written[tmp_ind]), A_NORMAL,
                            num_written_pair_f, NULL);
                    mvwchgat(wordlistwin, tmp_ind - (i - count), 
                            MAX_LIST_NAME_LENGTH + 2 + left / 3.0, 
                            get_len(wordlists->best_wpm[tmp_ind]), A_NORMAL,
                            best_wpm_pair_f, NULL);
                    mvwchgat(wordlistwin, tmp_ind - (i - count), 
                            MAX_LIST_NAME_LENGTH + 2 + left * (2/3.0), 
                            get_len((int)wordlists->avg_wpm[tmp_ind] + 1),
                            A_NORMAL, avg_wpm_pair_f, NULL);
                    mvwchgat(wordlistwin, tmp_ind - (i - count), 
                            MAX_LIST_NAME_LENGTH + 2 + left, 
                            get_len((int)wordlists->avg_acc[tmp_ind] + 1) + 3,
                            A_NORMAL, avg_acc_pair_f, NULL);
                }
            }
            mvwchgat(wordlistwin, off, 0, strlen(wordlists->lists[i - count +
                        off]) , A_REVERSE, 0, NULL);
            mvwchgat(wordlistwin, off, 
                    MAX_LIST_NAME_LENGTH + 2, 
                    get_len(wordlists->num_written[i - count + off]), A_BOLD,
                    num_written_pair_f, NULL);
            mvwchgat(wordlistwin, off, 
                    MAX_LIST_NAME_LENGTH + 2 + left / 3.0, 
                    get_len(wordlists->best_wpm[i - count + off]), A_BOLD,
                    best_wpm_pair_f, NULL);
            mvwchgat(wordlistwin, off,
                    MAX_LIST_NAME_LENGTH + 2 + left * (2/3.0),
                    get_len((int)wordlists->avg_wpm[i - count + off] + 1),
                    A_BOLD, avg_wpm_pair_f, NULL);
            mvwchgat(wordlistwin, off,
                    MAX_LIST_NAME_LENGTH + 2 + left, 
                    get_len((int)wordlists->avg_acc[i - count + off] + 1) + 3,
                    A_BOLD, avg_acc_pair_f, NULL);
            wrefresh(wordlistwin);
        }
        move(WL_YSTART + off, 5);
        wrefresh(paramwin);
    }
    if(indices_initialized){
        free(indices->content);
        free(indices);
    }
    mvaddch(2, max_x - 16, ACS_HLINE);
    werase(wordlistwin);
    delwin(wordlistwin);
    werase(paramwin);
    delwin(paramwin);
    if(modified){
        if(thread_init){
            forced_end = 1;
            sem_post(&lock);
            pthread_join(thread, NULL);
        }
        sem_close(&lock);
        repeat();
        exit(0);
    }
}

void repeat(){
    cur_words = 0;
    cur_secs = 0;
    type_x = 0;
    type_y = 0;
    sem_destroy(&lock);
    wmove(typewin, type_y, type_x);
    werase(clockwordwin);
    werase(titlewin);
    werase(printwin);
    werase(infowin);
    werase(progresswin);
    werase(typewin);
    werase(optionwin);
    werase(statwin);
    werase(lrwin);
    werase(keyboardwin);
    werase(fingerwin);
    erase();
    refresh();
    play(init_words, init_secs);
}

void print_options(WINDOW *win, char options[][MAX_OPT_SIZE], int start_y,
        int start_x, int len, short color){
    int i, x;

    x = start_x;
    for(i = 0; i < len; i++){
        if(x + strlen(options[i]) + 2 > max_x - 1){
            x = start_x;
            start_y += 1;
        }
        mvwprintw(win, start_y, x, "[%s] ", options[i]);
        mvwchgat(win, start_y, x + 1, strlen(options[i]), A_NORMAL, color,
                NULL);
        x += strlen(options[i]) + 3;
    }
}

void pause_time(){
    WINDOW *tmpwin;
    char c;
    char pauseopts[4][MAX_OPT_SIZE] = {{"Continue: 'c'"}, {"Restart: 'r'"},
        {"Switch to Paramwindow: 'p'"}, {"Quit: 'q'"}};
    char typeopts[1][MAX_OPT_SIZE] = {{"Pause: 'Esc'"}};
    char endopts[3][MAX_OPT_SIZE] = {{"Continue: 'c'"}, {"Switch to Paramwindow" 
        ": 'p'"}, {"Quit: 'q'"}};
    char paramopts[13][MAX_OPT_SIZE] = {{"Continue: 'c'"}, {"Go Up: 'up'/'k'"},
        {"Go Down: 'down'/'j'"}, {"Select: 'Enter'"}, {"Toggle Mode: 'm'"},
        {"Toggle Advance-Mode: 'a'"}, {"Change Time: 't'"}, {"Write File: 'w'"},
        {"Search: '/'"}, {"Target Next: 'n'"}, {"Target Previous: 'N'"},
        {"Jump to Start: 'g'"}, {"Jump to End: 'G'"}};

    tmpwin = newwin(max_y - 4, max_x, 3, 0);

    mvwvline(tmpwin, 0, 12, 0, max_y - 4);
    mvwhline(tmpwin, 1, 0, 0, max_x);
    mvwhline(tmpwin, (max_y - 6) / 4.0 + 2, 0, 0, max_x);
    mvwhline(tmpwin, (max_y - 6) / 4.0 * 2 + 2, 0, 0, max_x);
    mvwhline(tmpwin, (max_y - 6) / 4.0 * 3 + 2, 0, 0, max_x);
    mvaddch(2, 12, ACS_TTEE);
    mvwaddch(tmpwin, 1, 12, ACS_PLUS);
    mvwaddch(tmpwin, (max_y - 6) / 4.0 + 2, 12, ACS_PLUS);
    mvwaddch(tmpwin, (max_y - 6) / 4.0 * 2 + 2, 12, ACS_PLUS);
    mvwaddch(tmpwin, (max_y - 6) / 4.0 * 3 + 2, 12, ACS_PLUS);

    mvwprintw(tmpwin, 0, 0, "Window");
    mvwprintw(tmpwin, 0, 13, "Commands");
    mvwprintw(tmpwin, 2, 0, "PAUSEWINDOW");
    mvwprintw(tmpwin, (max_y - 6) / 4.0 + 3, 0, "TYPEWINDOW");
    mvwprintw(tmpwin, (max_y - 6) / 4.0 * 2 + 3, 0, "ENDWINDOW");
    mvwprintw(tmpwin, (max_y - 6) / 4.0 * 3 + 3, 0, "PARAMWINDOW");

    mvwchgat(tmpwin, 0, 0, 6, A_BOLD, 0, NULL);
    mvwchgat(tmpwin, 0, 13, 8, A_BOLD, 0, NULL);
    mvwchgat(tmpwin, 2, 0, 11, A_BOLD, pausewin_pair_f, NULL);
    mvwchgat(tmpwin, (max_y - 6) / 4.0 + 3, 0, 10, A_BOLD, typewin_pair_f,
            NULL);
    mvwchgat(tmpwin, (max_y - 6) / 4.0 * 2 + 3 , 0, 9, A_BOLD, endwin_pair_f,
            NULL);
    mvwchgat(tmpwin, (max_y - 6) / 4.0 * 3 + 3, 0, 11, A_BOLD, paramwin_pair_f,
            NULL);

    print_options(tmpwin, pauseopts, 2, 13, 4, pausewin_pair_f);
    print_options(tmpwin, typeopts, (max_y - 6) / 4.0 + 3, 13, 1,
            typewin_pair_f);
    print_options(tmpwin, endopts, (max_y - 6) / 4.0 * 2 + 3, 13, 3,
            endwin_pair_f);
    print_options(tmpwin, paramopts, (max_y - 6) / 4.0 * 3 + 3, 13, 13,
            paramwin_pair_f);

    werase(optionwin); 
    wattr_set(optionwin, A_BOLD, pausewin_pair_f, NULL);
    mvwprintw(optionwin, 0, 0, "[PAUSEWINDOW]");
    mvwprintw(optionwin, 0, max_x - 28, "[SWITCH TO PARAMWINDOW: 'p']");
    wrefresh(optionwin);
    wrefresh(tmpwin);
    move(3 + (max_y - 4) / 5.0, 3);
    refresh();
    while((c = getchar()) != 'c' && c != 'r' && c != 'p' && c != 'q')
        ;
    if(c == 'c')
        ;
    else if(c == 'r'){
        if(thread_init){
            forced_end = 1;
            sem_post(&lock);
            pthread_join(thread, NULL);
        }
        sem_close(&lock);
        repeat();
        exit(0);
    }
    else if(c == 'p'){
        mvaddch(2, 12, ACS_HLINE);
        change_params();
        mvaddch(2, 12, ACS_PLUS);
    }else{
        if(thread_init){
            forced_end = 1;
            sem_post(&lock);
            pthread_join(thread, NULL);
        }
        sem_close(&lock);
        cleanup();
        exit(0);
    }
    werase(optionwin);
    mvaddch(2, 12, ACS_HLINE);
    wattr_set(optionwin, A_BOLD, typewin_pair_f, NULL);
    mvwprintw(optionwin, 0, 0, "[TYPEWINDOW]");
    mvwprintw(optionwin, 0, max_x - 14, "[PAUSE: 'Esc']");
    refresh_screen();
    redrawwin(lrwin);
    redrawwin(keyboardwin);
    redrawwin(fingerwin);
    wrefresh(lrwin);
    wrefresh(keyboardwin);
    wrefresh(fingerwin);
}

void register_keystroke(char c, int err, struct total_t *total,
        struct error_t *errors){
    if(c == ' '){
        total->thumb++;
        if(err)
            errors->thumb++;
    }else if(c == 'z' || c == 'Z' || c == 'a' || c == 'A' || c == 'q' ||
            c == 'Q' || c == '1'  || c == '!'|| c == '2' || c == '@'){
        total->l_little++;
        if(err)
            errors->l_little++;
    }else if(c == 'x' || c == 'X' || c == 's' || c == 'S' || c == 'w' || 
            c == 'W' || c == '3' || c == '#'){
        total->l_ring++;
        if(err)
            errors->l_ring++;
    }else if(c == 'c' || c == 'C' || c == 'd' || c == 'D' || c == 'e' ||
            c == 'E' || c == '4' || c == '$'){
        total->l_middle++;
        if(err)
            errors->l_middle++;
    }else if(c == 'v' || c == 'V' || c == 'b' || c == 'B' || c == 'f' ||
            c == 'F' || c == 'g' || c == 'G' || c == 'r' || c == 'R' ||
            c == 't' || c == 'T' || c == '5' || c == '%' || c == '6' ||
            c == '^'){
        total->l_index++;
        if(err)
            errors->l_index++;
    }else if(c == 'n' || c == 'N' || c == 'm' || c == 'M' || c == 'h' ||
            c == 'H' || c == 'j' || c == 'J' || c == 'y' || c == 'Y' ||
            c == 'u' || c == 'U' || c == '7' || c == '&' || c == '8' || 
            c == '*'){
        total->r_index++;
        if(err)
            errors->r_index++;
    }else if(c == ',' || c == '<' || c == 'k' || c == 'K' || c == 'i' ||
            c == 'I' || c == '9' || c == '('){
        total->r_middle++;
        if(err)
            errors->r_middle++;
    }else if(c == '.' || c == '>' || c == 'l' || c == 'L' || c == 'o' || 
            c == 'O' || c == '0' || c == ')' || c == '-' || c == '_'){
        total->r_ring++;
        if(err)
            errors->r_ring++;
    }else{
        total->r_little++;
        if(err)
            errors->r_little++;
    }
}

int timed(){
    return mode;
}

int iterative_play(){
    return iterative;
}

int end_criterium_reached(){
    if(timed())
        return init_secs - cur_secs <= 0;
    return cur_words == init_words;
}

void update_progress(){
    while((progress += progress_step) <= (timed() ? cur_secs :
                cur_words) + EPSILON)
        wprintw(progresswin, progress_char);
    progress -= progress_step;
    wrefresh(progresswin);
}

void *clockword_thread(void *arg){
    struct timeval startt, currenttime;

    thread_init = 1;
    gettimeofday(&startt, NULL);
    starttime = startt.tv_sec + (double)startt.tv_usec / MICROSEC_T;
    while(sem_wait(&lock) || !forced_end){
        gettimeofday(&currenttime, NULL);
        cur_secs = (currenttime.tv_sec  + (double)currenttime.tv_usec /
                MICROSEC_T) - starttime;
        update_progress();
        if(end_criterium_reached()){
            mvwprintw(clockwordwin, 0, 0, "[%6d|%6.1f]", timed() ? cur_words : 
                init_words - cur_words, timed() ? 0 : cur_secs);
            wrefresh(clockwordwin);
            break;
        }
        mvwprintw(clockwordwin, 0, 0, "[%6d|%6.1f]", timed() ? cur_words : 
            init_words - cur_words, timed() ? init_secs - cur_secs : cur_secs);
        wrefresh(clockwordwin);
        refresh();
        sem_post(&lock);
        napms(100);
    }
    sem_post(&lock);
    end = 1;
    pthread_exit(NULL);
}

void check(struct indices_t *indices, struct total_t *total,
        struct error_t *errors){
    int i, curlen, word_x, cur_exp_c, set, cur_errors;
    char c, *expected_word, *typed_word;
    struct timeval currenttime, pausetime;

    word_x = set = 0;
    if(total->chars != 0)
        set = 1;
    for(i = 0; i < indices->count; i++){
        curlen = cur_exp_c = cur_errors = 0;
        expected_word = wordlist[indices->content[i]];
        typed_word = malloc((wordlength[indices->content[i]] + 1) *
                sizeof(char));
        if(set)
            sem_wait(&lock);
        mvwchgat(printwin, 0, word_x, wordlength[indices->content[i]], 
                A_UNDERLINE, 0, NULL);
        wrefresh(printwin);
        if(type_x + wordlength[indices->content[i]] > PT_XLENGTH){
            waddch(typewin, '\n');
            type_x = 0;
            type_y += type_y >= max_y - 24 ? 0 : 1;
            wrefresh(typewin);
        }
        color_keyboard_key(expected_word[0]);
        if(set)
            sem_post(&lock);
        while(((c = getch()) != '\n' && c != '\t' && c != ' ') || curlen == 0){
            if(end){
                pthread_join(thread, NULL);
                errors->chars_in_words += cur_errors > 0 ? (curlen - cur_errors)
                    : 0;
                free(typed_word);
                sem_close(&lock);
                return;
            }
            if(!set && c != ESCAPE){
                set = 1;
                pthread_create(&thread, NULL, &clockword_thread, NULL);
            }
            if(c == ESCAPE){
                sem_wait(&lock);
                gettimeofday(&currenttime, NULL);
                pause_time();
                gettimeofday(&pausetime, NULL);
                starttime += (pausetime.tv_sec - currenttime.tv_sec) + 
                    ((double)(pausetime.tv_usec - currenttime.tv_usec) /
                     MICROSEC_T);
                sem_post(&lock);
            }else if(c == 127){
                if(curlen > 0){
                    if(curlen == cur_exp_c)
                        cur_exp_c--;
                    sem_wait(&lock);
                    wmove(typewin, type_y, type_x - 1);
                    wdelch(typewin); 
                    curlen--;
                    type_x--;
                    if(curlen > cur_exp_c)
                        color_keyboard_key('\b');
                    else if(cur_exp_c == wordlength[indices->content[i]])
                        color_keyboard_key(' ');
                    else
                        color_keyboard_key(expected_word[cur_exp_c]);
                    sem_post(&lock);
                }
            }else if(c == '\n' || c == '\t' || c == ' '){
                continue;
            }
            else{
                sem_wait(&lock);
                if(type_x < PT_XLENGTH)
                    mvwaddch(typewin, type_y, type_x, c);
                sem_post(&lock);
                if(curlen < wordlength[indices->content[i]]){
                    typed_word[curlen] = c;
                    if(c != expected_word[curlen]){
                        sem_wait(&lock);
                        color_keyboard_key('\b');
                        sem_post(&lock);
                        errors->chars++;
                        cur_errors++;
                        register_keystroke(c, 1, total, errors);
                    }else{
                        if(curlen == cur_exp_c){
                            sem_wait(&lock);
                            mvwchgat(printwin, 0, word_x + cur_exp_c++, 1,
                                    A_BOLD, 0, NULL);
                            wrefresh(printwin);
                            if(cur_exp_c == wordlength[indices->content[i]])
                                color_keyboard_key(' ');
                            else
                                color_keyboard_key(expected_word[cur_exp_c]);
                            sem_post(&lock);
                            register_keystroke(c, 0, total, NULL);
                        }else{
                            errors->chars++;
                            cur_errors++;
                            register_keystroke(c, 1, total, errors);
                        }
                    }
                }else{
                    sem_wait(&lock);
                    color_keyboard_key('\b');
                    sem_post(&lock);
                    errors->chars++;
                    cur_errors++;
                    register_keystroke(c, 1, total, errors);
                }
                total->chars++;
                if(type_x < PT_XLENGTH){
                    curlen++;
                    type_x++;
                }
            }
            sem_wait(&lock);
            refresh();
            wrefresh(typewin);
            sem_post(&lock);
        }
        sem_wait(&lock);
        cur_words++;
        sem_post(&lock);
        total->words++;
        total->chars++;
        typed_word[wordlength[indices->content[i]]] = '\0';
        if(type_x < PT_XLENGTH){
            sem_wait(&lock);
            mvwaddch(typewin, type_y, type_x, ' ');
            sem_post(&lock);
        }
        if(curlen != wordlength[indices->content[i]] ||  
                strcmp(expected_word, typed_word) != 0){
            sem_wait(&lock);
            mvwchgat(printwin, 0, word_x, wordlength[indices->content[i]],
                    A_NORMAL, red_f, NULL);
            sem_post(&lock);
            register_keystroke(' ', 1, total, errors);
            errors->words++;
            errors->chars++;
            errors->chars_in_words += (curlen - cur_errors);
        }else{
            sem_wait(&lock);
            mvwchgat(printwin, 0, word_x, wordlength[indices->content[i]],
                    A_NORMAL, green_f, NULL);
            sem_post(&lock);
            register_keystroke(' ', 0, total, NULL);
        }
        sem_wait(&lock);
        wrefresh(typewin);
        wrefresh(printwin);
        refresh();
        wpm = (double)(total->chars - errors->chars - errors->chars_in_words) /
            cur_secs / 5 * 60;
        sem_post(&lock);
        if(cur_secs >= 3 || cur_words >= 3)
            best_wpm = wpm > best_wpm ? wpm : best_wpm;
        type_x++;
        word_x += wordlength[indices->content[i]] + 1;
        free(typed_word);
    }
    if(end_criterium_reached()){
        pthread_join(thread, NULL);
        sem_close(&lock);
    }
}

void play_timed_random(struct total_t *total, struct error_t *errors){
    int started;
    struct indices_t *indices1, *indices2;

    started = 0;
    wattr_set(clockwordwin, A_BOLD, timed_random_pair_f, NULL);
    wattr_set(progresswin, A_NORMAL, timed_random_pair_f, NULL);
    wattr_set(infowin, A_BOLD, timed_random_pair_f, NULL);
    mvwprintw(infowin, 0, 0, "[TIMED|RANDOM]");
    mvwprintw(clockwordwin, 0, 0, "[%6d|%6.1f]", 0, (double)init_secs);
    wrefresh(clockwordwin);
    wrefresh(infowin);
    indices1 = get_rand_wordindices();        
    print_words(indices1, 0);
    while(!end){
        indices2 = get_rand_wordindices();
        if(started && !end){
            wmove(printwin, 0, 0);
            wdeleteln(printwin);
        }
        print_words(indices2, 1);
        check(indices1, total, errors);
        free(indices1->content);
        free(indices1);
        indices1 = get_rand_wordindices();
        wmove(printwin, 0, 0);
        if(!end){
            wdeleteln(printwin);
            print_words(indices1, 1);
            check(indices2, total, errors);
        }
        free(indices2->content);
        free(indices2);
        started++;
    }
    free(indices1->content);
    free(indices1);
}

void play_timed_iterative(int words, struct total_t *total,
        struct error_t *errors){
    int started;
    struct indices_t *indices1, *indices2;

    started = 0;
    wattr_set(clockwordwin, A_BOLD, timed_iterative_pair_f, NULL);
    wattr_set(progresswin, A_NORMAL, timed_iterative_pair_f, NULL);
    wattr_set(infowin, A_BOLD, timed_iterative_pair_f, NULL);
    mvwprintw(infowin, 0, 0, "[TIMED|ITERATIVE]");
    mvwprintw(clockwordwin, 0, 0, "[%6d|%6.1f]", 0, (double)init_secs);
    wrefresh(clockwordwin);
    wrefresh(infowin);
    indices1 = get_iter_wordindices(words);        
    words -= indices1->count;
    words = words == 0 ? init_words : words;
    print_words(indices1, 0);
    while(!end){
        indices2 = get_iter_wordindices(words);
        words -= indices2->count;
        words = words == 0 ? init_words : words;
        if(started && !end){
            wmove(printwin, 0, 0);
            wdeleteln(printwin);
        }
        print_words(indices2, 1);
        check(indices1, total, errors);
        free(indices1->content);
        free(indices1);
        indices1 = get_iter_wordindices(words);
        words -= indices1->count;
        words = words == 0 ? init_words : words;
        wmove(printwin, 0, 0);
        if(!end){
            wdeleteln(printwin);
            print_words(indices1, 1);
            check(indices2, total, errors);
        }
        free(indices2->content);
        free(indices2);
        started++;
    }
    free(indices1->content);
    free(indices1);
}

void play_consume_random(int words, struct total_t *total,
        struct error_t *errors){
    int started, i;
    struct wordinfo_t *info;
    struct indices_t *indices1, *indices2;

    info = malloc(sizeof(*info));
    info->length = words;
    info->indices = malloc(sizeof(int) * words);
    for(i = 0; i < words; i++)
        info->indices[i] = i;
    started = 0;
    wattr_set(clockwordwin, A_BOLD, consume_random_pair_f, NULL);
    wattr_set(progresswin, A_NORMAL, consume_random_pair_f, NULL);
    wattr_set(infowin, A_BOLD, consume_random_pair_f, NULL);
    mvwprintw(infowin, 0, 0, "[CONSUME|RANDOM]");
    mvwprintw(clockwordwin, 0, 0, "[%6d|%6.1f]", init_words, 0.0);
    wrefresh(clockwordwin);
    wrefresh(infowin);
    indices1 = get_consume_rand_wordindices(info);
    words -= indices1->count;
    print_words(indices1, 0);
    while(!end){
        indices2 = get_consume_rand_wordindices(info);    
        words -= indices2->count;
        if(started && !end){
            wmove(printwin, 0, 0);
            wdeleteln(printwin);
        }
        print_words(indices2, 1);
        check(indices1, total, errors);
        free(indices1->content);
        free(indices1);
        indices1 = get_consume_rand_wordindices(info);
        words -= indices1->count;
        wmove(printwin, 0, 0);
        if(!end){
            wdeleteln(printwin);
            print_words(indices1, 1);
            check(indices2, total, errors);
        }
        free(indices2->content);
        free(indices2);
        started++;
    }
    free(indices1->content);
    free(indices1);
    free(info->indices);
    free(info);
}

void play_consume_iterative(int words , struct total_t *total,
        struct error_t *errors){
    int started;
    struct indices_t *indices1, *indices2;

    started = 0;
    wattr_set(clockwordwin, A_BOLD, consume_iterative_pair_f, NULL);
    wattr_set(progresswin, A_NORMAL, consume_iterative_pair_f, NULL);
    wattr_set(infowin, A_BOLD, consume_iterative_pair_f, NULL);
    mvwprintw(infowin, 0, 0, "[CONSUME|ITERATIVE]");
    mvwprintw(clockwordwin, 0, 0, "[%6d|%6.1f]", init_words, 0.0);
    wrefresh(clockwordwin);
    wrefresh(infowin);
    indices1 = get_iter_wordindices(words);
    words -= indices1->count;
    print_words(indices1, 0);
    while(!end){
        indices2 = get_iter_wordindices(words);    
        words -= indices2->count;
        if(started && !end){
            wmove(printwin, 0, 0);
            wdeleteln(printwin);
        }
        print_words(indices2, 1);
        check(indices1, total, errors);
        free(indices1->content);
        free(indices1);
        indices1 = get_iter_wordindices(words);
        words -= indices1->count;
        wmove(printwin, 0, 0);
        if(!end){
            wdeleteln(printwin);
            print_words(indices1, 1);
            check(indices2, total, errors);
        }
        free(indices2->content);
        free(indices2);
        started++;
    }
    free(indices1->content);
    free(indices1);
}

void statwin_add_blanks(double num){
    num = num < 1 ? 1 : num;
    for(; num < 100; num *= 10)
        waddch(statwin, ' ');
    waddch(statwin, ' ');
}

void print_stats(struct total_t *total, struct error_t *errors){
    double tmp_percent, wpm;

    if(timed())
        wpm = (double)(total->chars - errors->chars - errors->chars_in_words) /
            5 / init_secs * 60;
    else
        wpm = (double)(total->chars - errors->chars - errors->chars_in_words) /
            5 / cur_secs * 60;
    mvwprintw(statwin, 1, 2, " Words per Minute : %.0f (%d/%d)", wpm,
            total->words - errors->words, total->words);
    mvwchgat(statwin, 1, 2, 49, A_BOLD, 0, NULL);
    mvwprintw(statwin, 2, 2, " Best Wpm         : %.0f", best_wpm == 0 ? wpm :
            best_wpm);
    mvwhline(statwin, 3, 2, 0, 49);

    tmp_percent = total->chars == 0 ? 100 :
        (double)(total->chars - errors->chars) / total->chars * 100;
    mvwprintw(statwin, 4, 2, " Total Accuracy   : %.2f", tmp_percent);
    waddch(statwin, '%');
    wprintw(statwin, " (%d/%d)", total->chars - errors->chars, total->chars);
    mvwchgat(statwin, 4, 2, 49, A_BOLD, 0, NULL);

    tmp_percent = errors->thumb == 0 ? 100 : total->thumb == 0 ? 0 :
        (double)(total->thumb - errors->thumb) / total->thumb * 100;
    mvwprintw(statwin, 5, 2, " Accuracy[Thumb]  : %.2f%", tmp_percent);
    waddch(statwin, '%');
    mvwchgat(statwin, 5, 22, 7, A_NORMAL, thumb_pair_f, NULL);

    tmp_percent = errors->l_index == 0 ? 100 : total->l_index == 0 ? 0 :
        (double)(total->l_index - errors->l_index) / total->l_index * 100;
    mvwprintw(statwin, 6, 2, " Accuracy[Index]  : left: %.2f", tmp_percent);
    waddch(statwin, '%');
    statwin_add_blanks(tmp_percent);
    tmp_percent = errors->r_index == 0 ? 100 : total->r_index == 0 ? 0 :
        (double)(total->r_index - errors->r_index) / total->r_index * 100;
    wprintw(statwin, "right: %.2f", tmp_percent);
    waddch(statwin, '%');
    mvwchgat(statwin, 6, 22, 13, A_NORMAL, index_left_pair_f, NULL);
    mvwchgat(statwin, 6, 36, 14, A_NORMAL, index_right_pair_f, NULL);

    tmp_percent = errors->l_middle == 0 ? 100 : total->l_middle == 0 ? 0 :
        (double)(total->l_middle - errors->l_middle) / total->l_middle  * 100;
    mvwprintw(statwin, 7, 2, " Accuracy[Middle] : left: %.2f", tmp_percent);
    waddch(statwin, '%');
    statwin_add_blanks(tmp_percent);
    tmp_percent = errors->r_middle == 0 ? 100 : total->r_middle == 0 ? 0 :
        (double)(total->r_middle - errors->r_middle) / total->r_middle * 100;
    wprintw(statwin, "right: %.2f", tmp_percent);
    waddch(statwin, '%');
    mvwchgat(statwin, 7, 22, 13, A_NORMAL, middle_left_pair_f, NULL);
    mvwchgat(statwin, 7, 36, 14, A_NORMAL, middle_right_pair_f, NULL);

    tmp_percent = errors->l_ring == 0 ? 100 : total->l_ring == 0 ? 0 :
        (double)(total->l_ring - errors->l_ring) / total->l_ring * 100;
    mvwprintw(statwin, 8, 2, " Accuracy[Ring]   : left: %.2f", tmp_percent);
    waddch(statwin, '%');
    statwin_add_blanks(tmp_percent);
    tmp_percent = errors->r_ring == 0 ? 100 : total->r_ring == 0 ? 0 :
        (double)(total->r_ring - errors->r_ring) / total->r_ring * 100;
    wprintw(statwin, "right: %.2f", tmp_percent);
    waddch(statwin, '%');
    mvwchgat(statwin, 8, 22, 13, A_NORMAL, ring_left_pair_f, NULL);
    mvwchgat(statwin, 8, 36, 14, A_NORMAL, ring_right_pair_f, NULL);

    tmp_percent = errors->l_little == 0 ? 100 : total->l_little == 0 ? 0 :
        (double)(total->l_little - errors->l_little) / total->l_little  * 100;
    mvwprintw(statwin, 9, 2, " Accuracy[Little] : left: %.2f", tmp_percent);
    waddch(statwin, '%');
    statwin_add_blanks(tmp_percent);
    tmp_percent = errors->r_little == 0 ? 100 : total->r_little == 0 ? 0 :
        (double)(total->r_little - errors->r_little) / total->r_little * 100;
    wprintw(statwin, "right: %.2f", tmp_percent);
    waddch(statwin, '%');
    mvwchgat(statwin, 9, 22, 13, A_NORMAL, little_left_pair_f, NULL);
    mvwchgat(statwin, 9, 36, 14, A_NORMAL, little_right_pair_f, NULL);
}

void play(int words, time_t secs){
    char c;
    struct error_t *errors; 
    struct total_t *total;

    sem_init(&lock, 0, 1);
    best_wpm = end = forced_end = thread_init = cur_secs = cur_words = progress
        = 0;
    errors = malloc(sizeof(*errors));
    total = malloc(sizeof(*total));
    draw_full_keyboard();
    mvwprintw(titlewin, 0, 0, "<| 10type : %s |>", get_wordlist_name(list));
    wattr_set(optionwin, A_BOLD, typewin_pair_f, NULL);
    mvwprintw(optionwin, 0, 0, "[TYPEWINDOW]");
    mvwprintw(optionwin, 0, max_x - 14, "[PAUSE: 'Esc']");
    mvhline(2, 0, 0, max_x);
    mvaddch(6, PT_XSTART - 1, ACS_LTEE);
    mvhline(6, PT_XSTART, 0, PT_XLENGTH);
    mvaddch(6, PT_XLENGTH + PT_XSTART, ACS_RTEE);
    wmove(progresswin, 0, 0);
    wrefresh(titlewin);
    wrefresh(optionwin);
    total->words = total->chars = total->thumb = total->l_little = total->l_ring
        = total->l_middle = total->l_index = total->r_little = total->r_ring = 
        total->r_middle = total->r_index = 0;
    errors->words = errors->chars = errors->chars_in_words = errors->thumb =
        errors->l_little = errors->l_ring = errors->l_middle = errors->l_index =
        errors->r_little = errors->r_ring = errors->r_middle = errors->r_index
        = 0;
    if(timed()){
        if(iterative_play())
            play_timed_iterative(words, total, errors);
        else
            play_timed_random(total, errors);
    }else{
        if(iterative_play())
            play_consume_iterative(words, total, errors);
        else
            play_consume_random(words, total, errors);
    }
    undraw_full_keyboard();
    statwin = newwin(11, 53, max_y - 14, max_x / 2 - 26);
    werase(optionwin);
    wattr_set(optionwin, A_BOLD, endwin_pair_f, NULL);
    mvwprintw(optionwin, 0, 0, "[ENDWINDOW]");
    mvwprintw(optionwin, 0, max_x - 11, "[QUIT: 'q']");
    mvwhline(statwin, 0, 2, ACS_CKBOARD, 50);
    mvwhline(statwin, 10, 2, ACS_CKBOARD, 50);
    mvwvline(statwin, 0, 0, ACS_CKBOARD, 11);
    mvwvline(statwin, 0, 1, ACS_CKBOARD, 11);
    mvwvline(statwin, 0, 51, ACS_CKBOARD, 11);
    mvwvline(statwin, 0, 52, ACS_CKBOARD, 11);
    if(!timed() || init_secs >= 60)
        update_wordlist_stats(wordlists, list_index, timed() ? ((double)(
                total->chars - errors->chars - errors->chars_in_words) / 5 /
                init_secs * 60) : ((double)(total->chars - errors->chars - 
                errors->chars_in_words) / 5 / cur_secs * 60), total->chars == 0
                ? 100 : (double)(total->chars - errors->chars) / total->chars *
                100);
    print_stats(total, errors);
    move(type_y + 7, type_x + PT_XSTART);
    wrefresh(statwin);
    wrefresh(optionwin);
    refresh();
    free(errors);
    free(total);
    napms(500);
    flushinp();
    while((c = getch()) != 'q' && c != 'c'){
        if(c == 'p'){
            change_params();
            werase(optionwin);
            wattr_set(optionwin, A_BOLD, endwin_pair_f, NULL);
            mvwprintw(optionwin, 0, 0, "[ENDWINDOW]");
            mvwprintw(optionwin, 0, max_x - 11, "[QUIT: 'q']");
            refresh_screen();
            redrawwin(statwin);
            wrefresh(statwin);
        }
    }
    if(c == 'c')
        repeat();
    else if(c == 'q')
        cleanup();
}

void cleanup(){
    int i;

    sem_destroy(&lock);
    free(list);
    free(wordlength);    
    for(i = 0; i < init_words; i++)
        free(wordlist[i]);
    free(wordlist);

    for(i = 0; i < wordlists->num_entries; i++)
        free(wordlists->lists[i]);
    free(wordlists->lists);
    free(wordlists->num_written);
    free(wordlists->best_wpm);
    free(wordlists->avg_wpm);
    free(wordlists->avg_acc);
    free(wordlists);
    free(conf_location);
    free(list_location);
    free(tmp_location);
    free(stats_location);

    delwin(titlewin);
    delwin(typewin);
    delwin(printwin);
    delwin(clockwordwin);
    delwin(infowin);
    delwin(progresswin);
    delwin(keyboardwin);
    delwin(statwin);
    delwin(optionwin);
    endwin();
}

int main(int argv, char **argc){
    srand(time(NULL));
    setlocale(LC_ALL, "");
    location_init();
    set_params();
    set_wordlist(list);
    wordlists = get_avail_wordlists(list_location);
    initscr();
    start_color();
    use_default_colors();
    noecho();
    cbreak();
    curs_set(0);
    getmaxyx(stdscr, max_y, max_x);
    type_x = type_y = print_x = print_y = 0;
    def_colors();
    def_windows(strlen("<| 10type : ") + strlen(get_wordlist_name(list)) +
            strlen(" |>"));
    refresh();
    update_progress_step();
    play(init_words, init_secs);
}
