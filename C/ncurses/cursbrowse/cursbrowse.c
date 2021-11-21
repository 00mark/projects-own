#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <stdarg.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "cursbrowse.h"
#include "util.h"
#include "check_file.h"

int err, max_y, max_x;
short err_pair = 1, path_pair = 2, file_pair = 3, dir_pair = 4, search_pair = 5,
      link_file_pair = 6, link_dir_pair = 7, info_pair = 8;
char *prev_command, *info_txt;

file_matches *get_matches(char *path)
{
    file_matches *fm;
    FILE *fp;
    char tmp[CONFIG_LINE_LENGTH];
    int l1, l2, i, c;
    long off;

    fm = malloc(sizeof(*fm));
    fm->num_items = 0;
    if(!(fp = fopen(path, "r"))){
        err = ERR_CONFIG;
        return NULL;
    }
    while(fgets(tmp, CONFIG_LINE_LENGTH, fp)){
        while(strlen(tmp) == CONFIG_LINE_LENGTH)
            fgets(tmp, CONFIG_LINE_LENGTH, fp);
        fm->num_items++;
    }
    rewind(fp);
    fm->matches = malloc(sizeof(char **) * fm->num_items);
    for(i = 0; i < fm->num_items; i++){
        fm->matches[i] = malloc(sizeof(char *) * 2);
        l1 = l2 = 0; 
        off = ftell(fp);
        while(fgetc(fp) != ' ')
            l1++;
        while((c = fgetc(fp)) != EOF && c != '\n')
            l2++;
        fseek(fp, off, SEEK_SET);
        fm->matches[i][0] = malloc(l1 + 1);
        fm->matches[i][1] = malloc(l2 + 1);
        fscanf(fp, "%s %[^\nEOF]s", fm->matches[i][0], fm->matches[i][1]);
    }
    fclose(fp);

    return fm;
}

dir_files *get_files(char *path, int full)
{
    DIR *d;
    dir_files *df;
    struct stat path_stat;
    struct dirent *dir;
    int i, len;
    char *full_path, **tmp_files;

    df = malloc(sizeof(*df));
    df->num_items = df->max_uid_len = df->max_gid_len = i = 0;
    d = opendir(path);
    if(d){
        while((dir = readdir(d)) != NULL)
            if(strcmp(dir->d_name, "."))
                df->num_items++;
        rewinddir(d);
        tmp_files = malloc(sizeof(char *) * df->num_items);
        df->is_dir = malloc(sizeof(int) * df->num_items);
        df->is_link = malloc(sizeof(int) * df->num_items);
        if(full){
            df->perm = malloc(sizeof(char *) * df->num_items);
            df->gid = malloc(sizeof(gid_t) * df->num_items);
            df->uid = malloc(sizeof(uid_t) * df->num_items);
            df->size = malloc(sizeof(off_t) * df->num_items);
            df->last_access = malloc(sizeof(struct timespec) * df->num_items);
        }
        while((dir = readdir(d)) != NULL){
            if(strcmp(dir->d_name, ".")){
                len = strlen(dir->d_name);
                tmp_files[i] = malloc(len + 1);
                strcpy(tmp_files[i], dir->d_name);
                i++;
            }
        }
        closedir(d); 
        df->files = sort_files(tmp_files, df->num_items);
        if(df->num_items > 1)
            free(tmp_files);
        for(i = 0; i < df->num_items; i++){
            full_path = get_full_path(path, df->files[i]);
            lstat(full_path, &path_stat);
            if((df->is_link[i] = S_ISLNK(path_stat.st_mode)))
                df->is_dir[i] = test_path(full_path);
            else
                df->is_dir[i] = S_ISDIR(path_stat.st_mode);
            if(full){
                df->perm[i] = get_perm(path_stat.st_mode);
                df->gid[i] = path_stat.st_gid;
                df->uid[i] = path_stat.st_uid;
                df->size[i] = path_stat.st_size;
                df->last_access[i] = path_stat.st_atim;
                len = strlen(getpwuid(df->uid[i])->pw_name); 
                df->max_uid_len = (len > df->max_uid_len ? len :
                        df->max_uid_len);
                len = strlen(getgrgid(df->gid[i])->gr_name); 
                df->max_gid_len = (len > df->max_gid_len ? len :
                        df->max_gid_len);
            }
            free(full_path);
        }
    } 

    return df;
}

char *get_perm(mode_t mode)
{
    char *perms;
    int i, j;
    mode_t m1, m2, m3;

    i = 0;
    m1 = 0400;
    m2 = 0200;
    m3 = 0100;
    perms = malloc(11);
    perms[i++] = ((mode & 01000) == S_ISVTX ? 's' : '-');
    for(j = 0; j < 3; m1 = m1 >> 3, m2 = m2 >> 3, m3 = m3 >> 3, j++){
        perms[i++] = ((mode & m1) == (j == 0 ? S_IRUSR : j == 1 ? S_IRGRP :
                    S_IROTH) ? 'r' : '-');
        perms[i++] = ((mode & m2) == (j == 0 ? S_IWUSR : j == 1 ? S_IWGRP :
                    S_IWOTH) ? 'w' : '-');
        perms[i++] = ((mode & m3) == (j == 0 ? S_IXUSR : j == 1 ? S_IXGRP :
                    S_IXOTH) ? 'x' : '-');
    }
    perms[i] = '\0';

    return perms;
}

int test_path(char *path)
{
    DIR *d;
    struct dirent *dir;

    d = opendir(path);
    if(d){
        closedir(d);
        return 1;
    }
    return 0;
}

void cleanup_df(dir_files *df, int full)
{
    int i;

    for(i = 0; i < df->num_items; i++){
        free(df->files[i]);
        if(full)
            free(df->perm[i]);
    }
    free(df->files);
    free(df->is_dir);
    free(df->is_link);
    if(full){
        free(df->perm);
        free(df->gid);
        free(df->uid);
        free(df->size);
        free(df->last_access);
    }
    free(df);
}

void cleanup_fm(file_matches *fm)
{
    int i;

    for(i = 0; i < fm->num_items; i++){
        free(fm->matches[i][0]);
        free(fm->matches[i][1]);
        free(fm->matches[i]);
    }
    free(fm->matches);
    free(fm);
}

void cleanup_ils(index_location_stack *ils)
{
    int i;

    for(i = 0; i < ils->cur; i++)
        free(ils->content[i].path);
    free(ils->content);
    free(ils);
}

void show_err(int n)
{
    switch(err){
        case ERR_INPUT         : mvprintw(max_y - (1 + n), 0, "ERR: Unknown"
                                         " Input");
                                 break;
        case ERR_DIR           : mvprintw(max_y - (1 + n), 0, "ERR: No"
                                         " Directory");
                                 break;
        case ERR_PERMISSION    : mvprintw(max_y - (1 + n), 0, "ERR: No"
                                         " Permission");
                                 break;
        case ERR_CONFIG        : mvprintw(max_y - (1 + n), 0, "ERR: Could Not"
                                         " Open Config File");
                                 break;
        case ERR_START_PATH    : mvprintw(max_y - (1 + n), 0, "ERR: Could Not"
                                         " Open Start Path");
                                 break;
        case ERR_RECUR         : mvprintw(max_y - (1 + n), 0, "ERR: No Matches"
                                         " Found");
                                 break;
        case ERR_SEARCH        : mvprintw(max_y - (1 + n), 0, "ERR: Space");
                                 break;
        case ERR_COMMAND       : mvprintw(max_y - (1 + n), 0, "ERR: Cannot Run"
                                         " Command");
                                 break;
        case ERR_PREV_COMMAND  : mvprintw(max_y - (1 + n), 0, "ERR: No Previous"
                                         " Command");
                                 break;
        case ERR_PATH          : mvprintw(max_y - (1 + n), 0, "ERR: Path Does"
                                         " Not Exist");
                                 break;
        case ERR_MOUSE         : mvprintw(max_y - (1 + n), 0, "ERR: Mouse");
                                 break;
        default                : mvprintw(max_y - (1 + n), 0, "ERR: Unknown");
                                 break;
    }
    clrtoeol();
    mvchgat(max_y - (1 + n), 0, max_x, A_BOLD, err_pair, NULL);
}

void show_info(int n)
{
    if(info_txt != NULL){
        mvprintw(max_y - (1 + n), 0, "INFO: %s", info_txt);
        clrtoeol();
        mvchgat(max_y - (1 + n), 0, max_x, A_BOLD, info_pair, NULL);
    }
}

int is_index(int *indices, int i)
{
    int n;

    for(n = 0; indices[n] != END; n++){
        if(i == indices[n])
            return 1;
    }
    return 0;
}

void update_display(dir_files *df, char *path, int index, int print_ind, 
        int has_indices, int *indices)
{
    int  n, i, j, path_space, len, mode, max_uid_len, max_gid_len;
    char *tmp_str, *uid_name, *gid_name;

    erase();
    if(max_x < 40){
        mode = SIMPLE;
        len = max_x - 4;
    }else if(max_x < 85){
        mode = REDUCED;
        len = max_x - (27 + df->max_uid_len);
    }else{
        mode = FULL;
        len = max_x - (55 + df->max_gid_len + df->max_uid_len);
    }
    n = 0;
    j = print_ind;
    path_space = max_x - (3 + int_len(df->num_items) * 2);
    if(strlen(path) <= path_space)
        mvprintw(0, 0, path);
    else
        mvprintw(0, 0, "...%s", &(path[strlen(path) - path_space + 3]));
    mvprintw(0, path_space + 2, "%*d/%*d", int_len(df->num_items), index + 1,
            int_len(df->num_items), df->num_items);
    for(i = 0; j < df->num_items && i < max_y - 1; i++, j++){
        if(strlen(df->files[j]) > len){
            tmp_str = malloc(strlen(df->files[j]) + 1);
            strcpy(tmp_str, df->files[j]);
            tmp_str[len - 3] = '.';
            tmp_str[len - 2] = '.';
            tmp_str[len - 1] = '.';
            tmp_str[len] = '\0';
            mvprintw(i + 1, 0, "%s", tmp_str);
            free(tmp_str);
        }else
            mvprintw(i + 1, 0, "%s", df->files[j]);
        uid_name = getpwuid(df->uid[j])->pw_name;
        gid_name = getgrgid(df->gid[j])->gr_name; 
        if(mode == FULL)
            mvprintw(i + 1, len, "  %2s  %s  %*s  %*s  %6d%c  %s", 
                    (df->is_link[j] ?  df->is_dir[j] ? "LD" : "LF" :
                    df->is_dir[j] ? "D" : "F"), df->perm[j], df->max_uid_len,
                    uid_name, df->max_gid_len, gid_name, (df->size[j] >= 1024 *
                    1024 ?  df->size[j] / (1024 * 1024) : df->size[j] >= 1024 ?
                    df->size[j] / 1024 : df->size[j]), (df->size[j] >= 
                    1024 * 1024 ? 'M' : df->size[j] >= 1024 ? 'K' : 'B'),
                    ctime(&(df->last_access[j].tv_sec)));
        else if(mode == REDUCED)
            mvprintw(i + 1, len, "  %2s  %s  %*s  %6d%c", (df->is_link[j] ? 
                    df->is_dir[j] ? "LD" : "LF" : df->is_dir[j] ? "D" : "F"),
                    df->perm[j], df->max_uid_len, uid_name, (df->size[j] >=
                    1024 * 1024 ? df->size[j] / (1024 * 1024) : df->size[j] >=
                    1024 ?  df->size[j] / 1024 : df->size[j]), (df->size[j] >= 
                    1024 * 1024 ? 'M' : df->size[j] >= 1024 ? 'K' : 'B'));
        else
            mvprintw(i + 1, len, "  %2s", (df->is_link[j] ? df->is_dir[j] ? "LD"
                        : "LF" : df->is_dir[j] ? "D" : "F"));
        mvchgat(i + 1, 0, max_x, (has_indices ? is_index(indices, j) ? A_BOLD :
                A_NORMAL : A_NORMAL), (df->is_link[j] ? df->is_dir[j] ?
                link_dir_pair : link_file_pair : df->is_dir[j] ? dir_pair : 
                file_pair), NULL);
    }
    mvchgat(0, 0, max_x, A_BOLD | A_UNDERLINE, path_pair, NULL);
    mvchgat((index - print_ind < 0 ? (index - print_ind) * -1 + 1: index -
            print_ind + 1), 0, max_x, (has_indices ? is_index(indices, index) ? 
            A_BOLD | A_REVERSE : A_REVERSE : A_REVERSE), (df->is_link[index] ?
            df->is_dir[index] ? link_dir_pair : link_file_pair : 
            df->is_dir[index] ? dir_pair : file_pair), NULL);
    if(err){
        show_err(n);
        n++;
    }
    show_info(n);
    refresh();
}

char *update_path(char *path, char *f, int is_dir, int is_link)
{
    int i, len;
    char *cur_path;

    cur_path = malloc(strlen(path) + 1);
    strcpy(cur_path, path);
    if(!is_dir)
        err = ERR_DIR;
    else if(!strcmp(f, "."))
        ;
    else if(!strcmp(f, "..")){
        i = strlen(path) - 1;
        while(path[i] != '/')
            path[i--] = '\0';
        if(i != 0)
            path[i] = '\0';
    }else{
        path = realloc(path, strlen(path) + strlen(f) + 2);
        if(strcmp(path, "/"))
            strcat(path, "/");
        strcat(path, f);
    }

    if(test_path(path)){
        free(cur_path);
        return path;
    }
    if(is_link)
        err = ERR_DIR;
    else
        err = ERR_PERMISSION;
    free(path);
    return cur_path;
}

void def_colors()
{
    init_pair(err_pair, COLOR_WHITE, COLOR_RED);
    init_pair(path_pair, -1, -1);
    init_pair(file_pair, COLOR_YELLOW, -1);
    init_pair(dir_pair, COLOR_BLUE, -1);
    init_pair(search_pair, COLOR_WHITE, COLOR_YELLOW);
    init_pair(link_file_pair, COLOR_GREEN, -1);
    init_pair(link_dir_pair, COLOR_RED, -1);
    init_pair(info_pair, COLOR_YELLOW, COLOR_BLACK);
}

void *run_command(void *arg)
{
    system((char *) arg);
    free((char *) arg);
    pthread_exit(NULL);
}

char *create_command(char *fmt, ...)
{
    va_list ap;
    char *s;
    int size;

    size = 0;
    s = NULL;
    va_start(ap, fmt);
    size = vsnprintf(s, size, fmt, ap);
    va_end(ap);
    size++;
    s = malloc(size);
    va_start(ap, fmt);
    size = vsnprintf(s, size, fmt, ap);
    va_end(ap);
    if(info_txt != NULL)
        free(info_txt);
    info_txt = malloc(strlen(s) + 14); 
    strcpy(info_txt, "executed \"");
    strcpy(&(info_txt[10]), s);
    strcat(info_txt, "\"");

    return s;
}

int match(file_matches *fm, char *file_name, char *path)
{
    int i, len, file_type, m;
    char *full_path;

    len = strlen(file_name);
    for(i = 0; i < fm->num_items; i++){
        if(!(strcmp(fm->matches[i][0], "elf")))
            continue;
        if(len >= strlen(fm->matches[i][0])){
            if(!strcmp(fm->matches[i][0], &(file_name[len - 
                            strlen(fm->matches[i][0])])))
                return i;
        }
    }
    full_path = get_full_path(path, file_name);
    file_type = check(full_path);
    free(full_path);
    switch(file_type){
        case TEXT     : if((m = is_match(fm, "text")))
                            return m;
                        break;
        case ELF      : if((m = is_match(fm, "elf")))
                            return m;
                        break;
        case EXE      : if((m = is_match(fm, ".exe")))
                            return m;
                        break;
        case ISO      : if((m = is_match(fm, ".iso")))
                            return m;
                        break;
        case MKV_WEBM : if((m = is_match(fm, ".mkv")))
                            return m;
                        else if((m = is_match(fm, ".webm")))
                            return m;
                        break;
        case AVI      : if((m = is_match(fm, ".avi")))
                            return m;
                        break;
        case GIF      : if((m = is_match(fm, ".gif")))
                            return m;
                        break;
        case MPG      : if((m = is_match(fm, ".mpg")))
                            return m;
                        break;
        case FLAC     : if((m = is_match(fm, ".flac")))
                            return m;
                        break;
        case WAV      : if((m = is_match(fm, ".wav")))
                            return m;
                        break;
        case MP3      : if((m = is_match(fm, ".mp3")))
                            return m;
                        break;
        case PDF      : if((m = is_match(fm, ".pdf")))
                            return m;
                        break;
        case JPG      : if((m = is_match(fm, ".jpg")))
                            return m;
                        break;
        case PNG      : if((m = is_match(fm, ".png")))
                            return m;
                        break;
        case BMP      : if((m = is_match(fm, ".bmp")))
                            return m;
                        break;
        case TAR_GZ   : if((m = is_match(fm, ".tar.gz")))
                            return m;
                        else if((m = is_match(fm, ".gz")))
                            return m;
                        break;
        case ZIP      : if((m = is_match(fm, ".zip")))
                            return m;
                        break;
        case RAR      : if((m = is_match(fm, ".rar")))
                            return m;
                        break;
        case BZ2      : if((m = is_match(fm, ".bz2")))
                            return m;
                        break;
        case LZ       : if((m = is_match(fm, ".lz")))
                            return m;
                        break;
        case ZLIB     : if((m = is_match(fm, ".zlib")))
                            return m;
                        break;
    }
    return NO_MATCH;
}

int is_match(file_matches *fm, char *str)
{
    int i;

    for(i = 0; i < fm->num_items; i++){
        if(!strcmp(str, fm->matches[i][0]))
            return i;
    }
    return 0;
}

char *get_start_path(char **argv, int supplied)
{
    char *path, *pwd;

    if(supplied){
        if(argv[1][0] != '/'){
            pwd = getcwd(NULL, 0); 
            path = malloc(strlen(pwd) + strlen(argv[1]) + 2);
            strcpy(path, pwd);
            strcat(path, "/");
            strcat(path, argv[1]);
            free(pwd);
        }else{
            path = malloc(strlen(argv[1]) + 1);
            strcpy(path, argv[1]);
        }
        if(strcmp(path, "/") && path[strlen(path) - 1] == '/')
            path[strlen(path) - 1] = '\0';
        if(fopen(path, "r"))
            return path;
        else{
            free(path);
            err = ERR_START_PATH;
        }
    }
    return getcwd(NULL, 0);
}

char *get_command()
{
    WINDOW *search_win;
    int size, n, m, k, i, j, c, lim_x, cnt, back, first_arg, comp_len, *indices;
    char *cmd, *full_cmd, **cmds, **avail_commands;
    dir_files *cmd_files1, *cmd_files2;

    cmd_files1 = get_files(BIN_LOCATION_1, 0);
    cmd_files2 = get_files(BIN_LOCATION_2, 0);
    cmds = malloc(sizeof(char *) * (cmd_files1->num_items + 
                cmd_files2->num_items));
    indices = malloc(sizeof(int) * (cmd_files1->num_items +
                cmd_files2->num_items + 1));
    for(i = 0; i < cmd_files1->num_items; i++)
        cmds[i] = cmd_files1->files[i];
    for(j = 0; j < cmd_files2->num_items; j++)
        cmds[i + j] = cmd_files2->files[j];
    first_arg = 1;
    i = j = cnt = back = 0;
    lim_x = 2;
    if(max_y < 3 || max_x < 15 + (2 * lim_x))
        return NULL;
    size = CMD_SIZE;
    search_win = newwin(3, max_x - 1 - (2 * lim_x), (max_y - 1) / 2 - 1, 2);
    box(search_win, 0, 0);
    mvwprintw(search_win, 1, 1, "Run Command > ");
    mvwaddch(search_win, 1, 15, '_');
    mvwchgat(search_win, 1, 15, 1, A_BLINK | A_BOLD, 0, NULL);
    wrefresh(search_win);
    cmd = malloc(size);
    while((c = getch()) != ESC && c != '\n'){
        if(i > size - 3){
            size *= 2;
            cmd = realloc(cmd, size);
        }
        if(!first_arg && c == ' ')
            first_arg = 0;
        if(first_arg && c == '\t'){
            comp_len = n = m = k = 0;
            indices[0] = END;
            for(m = 0; m < cmd_files1->num_items + cmd_files2->num_items; m++){
                for(k = 0; k < strlen(cmd); k++){
                    if(cmds[m][k] != cmd[k])
                        break;
                }
                if(k == strlen(cmd))
                    indices[n++] = m;
            }
            indices[n] = END;
            if(n > 0){
                comp_len = strlen(cmds[indices[0]]); 
                for(m = 1; indices[m] != END; m++){
                    for(k = 0; k < strlen(cmds[indices[m]]) && k < comp_len &&
                            cmds[indices[m - 1]][k] == cmds[indices[m]][k]; k++)
                        ; 
                    comp_len = k;
                }
                if(comp_len > size - 2){
                    size *= 2;
                    cmd = realloc(cmd, size);
                }
                for(i = j = 0; i < comp_len; i++, j++)
                    cmd[i] = cmds[indices[0]][i];
                if(n == 1){
                    cmd[i++] = ' ';
                    j++;
                    first_arg = 0;
                }
                cmd[i] = '\0';
            }
        }else if(i != 0 && c == BSP){
            if(cmd[i - 1] == '%' && i >= 1){
                cmd[--i] = '\0';
            }else if(!first_arg && cmd[i - 1] == ' '){
                first_arg = 1;
                for(n = 0; n < i - 1; n++){
                    if(cmd[n] == ' '){
                        first_arg = 0;
                        break;
                    }
                }
            }
            cmd[--i] = '\0';
            j--;
            if((j + cnt > max_x - 18 - (2 * lim_x)) &&
                    (cmd[j + cnt - (max_x - 18 - (2 * lim_x))] == '%')){
                cnt--;
                back = 1;
            }
        }else if(c == '%'){
            cmd[i++] = c;
            cmd[i++] = c;
            cmd[i] = '\0';
            if(j + cnt >= max_x - 18 - (2 * lim_x))
                cnt = (cmd[j + cnt - (max_x - 18 - (2 * lim_x))] == '%' ?
                        cnt + 1 : cnt);
            j++;
            back = 0;
        }else if(c != BSP){
            cmd[i++] = c;
            cmd[i] = '\0';
            if(j + cnt >= max_x - 18 - (2 * lim_x))
                cnt = (cmd[j + cnt - (max_x - 18 - (2 * lim_x))] == '%' ?
                        cnt + 1 : cnt);
            j++;
            back = 0;
        }
        if(j + cnt <= max_x - 18 - (2 * lim_x))
            mvwprintw(search_win, 1, 15, cmd);
        else
            mvwprintw(search_win, 1, 15, &(cmd[j + cnt - (max_x - 18 - 
                                (2 * lim_x))]));
        wclrtoeol(search_win);
        mvwaddch(search_win, 1, max_x - 2 - (2 * lim_x), ACS_VLINE);
        mvwaddch(search_win, 1, (15 + j < max_x - 2 - (2 * lim_x) ?
                    15 + j : max_x - 3 - (2 * lim_x)), '_');
        mvwchgat(search_win, 1,  (15 + j < max_x - 2 - (2 * lim_x) ?
                    15 + j : max_x - 3 - (2 * lim_x)), 1,
                    A_BLINK | A_BOLD, 0, NULL);
        wrefresh(search_win);
    }
    cleanup_df(cmd_files1, 0);
    cleanup_df(cmd_files2, 0);
    free(cmds);
    free(indices);
    delwin(search_win); 
    if(c == ESC || !check_cmd(cmd)){
        free(cmd);
        return NULL;
    }
    full_cmd = malloc(strlen(cmd) + 10);
    strcpy(full_cmd, "urxvt -e ");
    strcat(full_cmd, cmd);
    free(cmd);
    if(prev_command != NULL)
        free(prev_command);
    prev_command = malloc(strlen(full_cmd) + 1);
    strcpy(prev_command, full_cmd);

    return full_cmd;
}

int check_cmd(char *cmd)
{
    int i, prev, found;
    char *tmp;

    for(i = found = prev = 0; cmd[i] != '\0'; i++){
        if(prev == '%' && cmd[i] == 's'){
            if(!found)
                found = i;
            else
                return 0;
        }
        prev = cmd[i];
    }
    if(!found)
        return 0;
    tmp = malloc(strlen(&(cmd[found])) + 1);
    strcpy(tmp, &(cmd[found]));
    strcpy(&(cmd[found - 1]), tmp);
    free(tmp);

    return 1; 
}

Window get_window()
{
    Window w;
    Display *dpy;
    Atom actual_type, filter_atom;
    int actual_format, status;
    unsigned long nitems, bytes_after;
    unsigned char *prop;

    dpy = XOpenDisplay(NULL);
    if(!dpy)
        return 0;
    filter_atom = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", True);
    status = XGetWindowProperty(dpy, RootWindow(dpy, XDefaultScreen(dpy)),
            filter_atom, 0, 1000, False, AnyPropertyType, &actual_type,
            &actual_format, &nitems, &bytes_after, &prop);
    w = prop[0] + (prop[1] << 8) + (prop[2] << 16) + (prop[3] << 24);
    XFree(prop);
    XCloseDisplay(dpy);

    return w;
}

void set_window_name(Window w, char *name)
{
    Display *dpy;

    dpy = XOpenDisplay(NULL);
    if(!dpy)
        return;
    XChangeProperty(dpy, w, XInternAtom(dpy, "_NET_WM_NAME", False),
        XInternAtom(dpy, "UTF8_STRING", False), 8, PropModeReplace, name,
        strlen(name));

    XCloseDisplay(dpy);
}

void toggle_index(int *indices, int *indices_init, int *n, int index,
        int num_items)
{
    int i, found;

    found = 0;

    if(!*indices_init){
        indices[0] = index;
        indices[1] = END;
        *indices_init = 1;
        *n = 0;
    }else{
        for(i = 0; indices[i] != END; i++){
            if(indices[i] == index){
                found = 1;
                break;
            }
        }
        if(found){
            *n--;
            while(indices[i] != END){
                indices[i] = indices[i + 1];
                i++;
            }
            if(indices[0] == END){
                *n = -1;
                if(*indices_init){
                    free(indices);
                    *indices_init = 0;
                }
            }
        }else{
            *n++;
            indices[i + 1] = END;
            while(i > 0 && indices[i - 1] > index){
                indices[i] = indices[i - 1];
                i--;
            }
            indices[i] = index;
        }
    }

}

int main(int argc, char **argv)
{
    dir_files *df;
    file_matches *fm;
    index_location_stack *ils_back, *ils_forward;
    index_location *il, tmp_il;
    char *path, *prev_path, *tmp_path, *search_str, *full_path, *command;
    char window_name[WINDOW_NAME_LENGTH + 1];
    int c, index, print_ind, indices_init, matches_init, i, j, n, end, start,
        max_search_len, search_c, start_index, match_index, exec_cmd, wrapped,
        c_updated, interval, len, *indices;
    pthread_t thread;
    mmask_t mask;
    MEVENT m_event;
    Window win;

    win = get_window();
    if(!win)
        exit(1);
    initscr();
    keypad(stdscr, TRUE);
    start_color();
    use_default_colors();
    curs_set(0);
    noecho();
    cbreak();
    getmaxyx(stdscr, max_y, max_x);
    def_colors();

    mask = mousemask(ALL_MOUSE_EVENTS, NULL);
    interval = mouseinterval(0);
    prev_command = NULL;
    info_txt = NULL;
    err = exec_cmd = c_updated = 0;
    ils_back = create_stack(DEFAULT_STACK_SIZE);
    ils_forward = create_stack(DEFAULT_STACK_SIZE);
    fm = get_matches(CONFIG_PATH);
    if(err)
        matches_init = 0;
    else
        matches_init = 1;
    c = -1; 
    index = print_ind = indices_init = start = end = 0;
    do{
        switch(c){
            case 'a'        : 
                            if(indices_init && indices[0] != END){
                                i = len = 0;
                                while(indices[i] != END){
                                    len += strlen(path) + strlen(
                                            df->files[indices[i]]) + 4;
                                    i++;
                                }
                                full_path = malloc(len + 1);
                                full_path[0] = '\0';
                                i = 0;
                                while(indices[i] != END){
                                    strcat(full_path, path);
                                    if(strcmp(path, "/"))
                                        strcat(full_path, "/");
                                    strcat(full_path, df->files[indices[i]]);
                                    if(indices[i+1] != END)
                                        strcat(full_path, "\" \"");
                                    i++;
                                }
                            }
            //FALLTHROUGH
            case '\n'       : 
                            if(matches_init && !df->is_dir[index] &&
                                    (match_index = match(fm, df->files[index],
                                    path)) != NO_MATCH){
                                if(c == '\n' || !indices_init || indices[0] ==
                                        END)
                                    full_path = get_full_path(path,
                                            df->files[index]);
                                if(!strcmp(fm->matches[match_index][0],
                                            ".tar.gz") || !strcmp(".zip",
                                            fm->matches[match_index][0])){
                                    tmp_path = malloc(strlen(path) + 1);
                                    strcpy(tmp_path, path);
                                    pthread_create(&thread, NULL, &run_command,
                                        create_command(
                                        fm->matches[match_index][1], full_path,
                                        tmp_path));
                                    free(tmp_path);
                                }else
                                    pthread_create(&thread, NULL, &run_command,
                                        create_command(
                                        fm->matches[match_index][1],
                                        full_path));
                                free(full_path);
                                pthread_detach(thread);
                                exec_cmd = 1;
                            }else if(c == '\n'){
                                tmp_il.index = index;
                                tmp_il.path = malloc(strlen(path) + 1);
                                strcpy(tmp_il.path, path);
                                push(ils_back, tmp_il);
                                path = update_path(path, df->files[index],
                                    df->is_dir[index], df->is_link[index]);
                                index = (!err ? 0 : index); print_ind =
                                    (!err ? 0 : print_ind);
                                if(!err && !empty(ils_forward))
                                    clr_stack(ils_forward);
                                cleanup_df(df, 1);
                                df = get_files(path, 1);
                                if(indices_init){
                                    free(indices);
                                    indices_init = 0;
                                }
                            }
                            break;
            case 'c'        : 
                            if((command = get_command()) != NULL){
                                full_path = get_full_path(path,
                                        df->files[index]);
                                pthread_create(&thread, NULL, &run_command,
                                    create_command(command, full_path));
                                pthread_detach(thread);
                                free(full_path);
                                free(command);
                                exec_cmd = 1;
                            }else
                                err = ERR_COMMAND;
                            break;
            case 'd'        :
            case 'D'        :
            case KEY_DOWN   :
            case 'j'        : 
            case 'J'        :
                            if(c != 'd' && c != 'D'){ 
                                if(c == 'J'){
                                    if(!indices_init)
                                        indices = calloc(df->num_items + 1,
                                                sizeof(int));
                                    toggle_index(indices, &indices_init, &n,
                                            index, df->num_items);
                                }
                                index += (index < df->num_items - 1 ? 1 : 0);
                            }else{
                                if(c == 'D'){
                                    for(i = 0; i < 5; i++){
                                        if(!indices_init)
                                            indices = calloc(df->num_items + 1,
                                                    sizeof(int));
                                        toggle_index(indices, &indices_init, &n,
                                                index, df->num_items);
                                        if(index == df->num_items - 1)
                                            break;
                                        index++;
                                    }
                                }else{
                                    index = (index + 5 < df->num_items ? index +
                                            5: df->num_items - 1);
                                }
                            }
                            while(index - print_ind > max_y - 2)
                                print_ind++;
                            if(indices_init && n != -1){
                                while(indices[n + 1] != END && index >= 
                                        indices[n + 1])
                                    n++;
                            }
                            wrapped = 0;
                            break;
            case 'u'        :
            case 'U'        :
            case KEY_UP     :
            case 'k'        : 
            case 'K'        :
                            if(c != 'u' && c != 'U'){
                                if(c == 'K'){
                                    if(!indices_init)
                                        indices = calloc(df->num_items + 1,
                                                sizeof(int));
                                    toggle_index(indices, &indices_init, &n,
                                            index, df->num_items);
                                }
                                index -= (index > 0 ? 1 : 0);
                            }else{
                                if(c == 'U'){
                                    for(i = 0; i < 5; i++){
                                        if(!indices_init)
                                            indices = calloc(df->num_items + 1,
                                                    sizeof(int));
                                        toggle_index(indices, &indices_init, &n,
                                                index, df->num_items);
                                        if(index == 0)
                                            break;
                                        index--;
                                    }
                                }else{
                                    index = (index - 5 >= 0 ? index - 5 : 0);
                                }
                            }
                            while(index < print_ind)
                                print_ind--;
                            if(indices_init && n != -1){
                                while(index < indices[n]){
                                    if(n == 0){
                                        n = 1;
                                        while(indices[n] != END)
                                            n++;
                                        n--;
                                        wrapped = 1;
                                        break;
                                    }else if(!wrapped)
                                        n--;
                                    else
                                        break;
                                }
                            }
                            break;
            case BSP        : 
                            tmp_il.index = index;
                            tmp_il.path = malloc(strlen(path) + 1);
                            strcpy(tmp_il.path, path);
                            push(ils_back, tmp_il);
                            path = (strcmp(path, "/") ? update_path(path, "..",
                                        1, 0) : path);
                            index = print_ind = 0;
                            cleanup_df(df, 1);
                            df = get_files(path, 1);
                            if(indices_init){
                                free(indices);
                                indices_init = 0;
                            }
                          break;
            case 'h'        : 
                            if(!empty(ils_back)){
                                tmp_il.index = index; tmp_il.path =
                                    malloc(strlen(path) + 1);
                                strcpy(tmp_il.path, path);
                                push(ils_forward, tmp_il);
                                tmp_il = pop(ils_back);
                                if(test_path(tmp_il.path)){
                                    free(path);
                                    path = malloc(strlen(tmp_il.path) + 1);
                                    strcpy(path, tmp_il.path);
                                    cleanup_df(df, 1);
                                    df = get_files(path, 1);
                                    index = (tmp_il.index < df->num_items ?
                                            tmp_il.index : df->num_items - 1);
                                    print_ind = (index > max_y - 1 ? index -
                                            (max_y - 2) : 0);
                                    free(tmp_il.path);
                                    if(indices_init){
                                        free(indices);
                                        indices_init = 0;
                                    }
                                }else{
                                    clr_stack(ils_back);
                                    err = ERR_PATH;
                                }
                            }
                            break;
            case 'l'        : 
                            if(!empty(ils_forward)){
                                tmp_il.index = index;
                                tmp_il.path = malloc(strlen(path) + 1);
                                strcpy(tmp_il.path, path);
                                push(ils_back, tmp_il);
                                tmp_il = pop(ils_forward);
                                if(test_path(tmp_il.path)){
                                    free(path);
                                    path = malloc(strlen(tmp_il.path) + 1);
                                    strcpy(path, tmp_il.path);
                                    cleanup_df(df, 1);
                                    df = get_files(path, 1);
                                    index = (tmp_il.index < df->num_items ?
                                            tmp_il.index : df->num_items - 1);
                                    print_ind = (index > max_y - 1 ? index -
                                            (max_y - 2) : 0);
                                    free(tmp_il.path);
                                    if(indices_init){
                                        free(indices);
                                        indices_init = 0;
                                    }
                                }else{
                                    clr_stack(ils_forward);
                                    err = ERR_PATH;
                                }
                            }
                            break;
            case 'g'        : 
                            print_ind = index = 0;
                            if(indices_init && n != -1){
                                if(indices[0] == 0)
                                    n = 0;
                                else{
                                    n = 1;
                                    while(indices[n] != END)
                                        n++;
                                    n--;
                                }
                            }
                            break;
            case 'G'        : 
                            print_ind = (max_y - 1 > df->num_items ? 0 :
                            df->num_items - (max_y - 1));
                            index = df->num_items - 1; 
                            if(indices_init && n != -1){
                                n = 1;
                                while(indices[n] != END)
                                    n++;
                                n--;
                            }
                            break;
            case '?'        :
            case '/'        : 
                            start_index = index;
                            if(max_x < 10){
                                err = ERR_SEARCH;
                                break;
                            }
                            max_search_len = max_x - 8;
                            search_str = malloc(max_search_len + 1);
                            i = 0;
                            search_str[0] = '\0';
                            mvprintw(max_y - 1, 0, "Search: ");
                            clrtoeol();
                            mvchgat(max_y - 1, 0, max_x, A_BOLD, search_pair, 
                                    NULL);
                            while((search_c = getch()) != '\n' &&
                                    search_c != ESC){
                                if(search_c == RESIZE)
                                    getmaxyx(stdscr, max_y, max_x);
                                else if(search_c == BSP){
                                    if(i > 0)
                                        search_str[--i] = '\0';
                                }else if(i < max_search_len){
                                    search_str[i++] = search_c;
                                    search_str[i] = '\0';
                                }
                                if(strlen(search_str)){
                                    if(indices_init)
                                        free(indices);
                                    indices = search_file(df->files,
                                            df->num_items, search_str, index,
                                            (c == '/' ? 0 : 1));
                                    indices_init = 1;
                                    n = -1;
                                 if(indices[0] != END){
                                        n = 0;
                                        index = indices[n];
                                        print_ind = (index > max_y - 2 ? index -
                                                (max_y - 3) : 0);
                                    }
                                } 
                                update_display(df, path, index, print_ind, 
                                        (indices_init ? 1 : 0), (indices_init ?
                                         indices : NULL));
                                move(max_y - 1, 0);
                                clrtoeol();
                                mvprintw(max_y - 1, 0, "Search: %s",
                                        search_str);
                                if(indices_init && n == -1){
                                    if(max_search_len - strlen(search_str) > 18)
                                        printw(" (No Results Found)");
                                    mvchgat(max_y - 1, 0, max_x, A_BOLD,
                                            err_pair, NULL);
                                }else
                                    mvchgat(max_y - 1, 0, max_x, A_BOLD,
                                            search_pair, NULL);
                            }
                            if(search_c == ESC){
                                index = start_index;
                                print_ind = (index > max_y - 2 ? index -
                                        (max_y - 3) : 0);
                                if(indices_init){
                                    free(indices);
                                    indices_init = 0;
                                }
                            }
                            free(search_str);
                          break;
            case 'n'        :
                            if(indices_init){
                                if(n != -1){
                                    n = (indices[n + 1] != END ? n + 1 : 0);
                                    index = indices[n];
                                    print_ind = (index > max_y - 2 ? index - 
                                                (max_y - 2) : 0);
                                }
                            }
                            wrapped = 0;
                            break;
            case 'N'        :
                            if(indices_init){
                                if(n != -1){
                                    if(index == indices[n]){
                                        if(n == 0){
                                            n = 1; 
                                            while(indices[n] != END)
                                                n++;
                                            n--;
                                        }else
                                            n--;
                                    }
                                    index = indices[n];
                                    print_ind = (index > max_y - 2 ? index - 
                                            (max_y - 2) : 0);
                                }
                            }
                            wrapped = 0;
                            break;
            case 'r'        : 
                            if(max_x < 20){
                                err = ERR_SEARCH;
                                break;
                            }
                            max_search_len = max_x - 18;
                            search_str = malloc(max_search_len + 1);
                            i = 0;
                            search_str[0] = '\0';
                            mvprintw(max_y - 1, 0, "Search Recursive: ");
                            clrtoeol();
                            mvchgat(max_y - 1, 0, max_x, A_BOLD, search_pair, 
                                    NULL);
                            while((search_c = getch()) != '\n' &&
                                    search_c != ESC){
                                if(search_c == RESIZE)
                                    getmaxyx(stdscr, max_y, max_x);
                                else if(search_c == BSP){
                                    if(i > 0)
                                        search_str[--i] = '\0';
                                }else if(i < max_search_len){
                                    search_str[i++] = search_c;
                                    search_str[i] = '\0';
                                }
                                mvprintw(max_y - 1, 0, "Search Recursive: %s",
                                        search_str);
                                clrtoeol();
                                mvchgat(max_y - 1, 0, max_x, A_BOLD,
                                        search_pair, NULL);
                            }
                            if(search_c != ESC){
                                il = search_file_recur(path, search_str);
                                if(!il)
                                    err = ERR_RECUR;
                                else{
                                    tmp_il.index = index;
                                    tmp_il.path = malloc(strlen(path) + 1);
                                    strcpy(tmp_il.path, path);
                                    push(ils_back, tmp_il);
                                    free(path);
                                    path = il->path;
                                    cleanup_df(df, 1);                                  
                                    df = get_files(path, 1);
                                    index = il->index;
                                    print_ind = (index > max_y - 2 ? index - 
                                            (max_y - 2) : 0);
                                    free(il);
                                    if(indices_init){
                                        free(indices);
                                        indices_init = 0;
                                    }
                                }
                            }
                            free(search_str);
                            break;
            /* excluded for now to avoid accidents */
            /* case 'p'        :  */
            /*                 if(prev_command != NULL){ */
            /*                     full_path = get_full_path(path, */
            /*                             df->files[index]); */
            /*                     command = malloc(strlen(prev_command) + 1); */
            /*                     strcpy(command, prev_command); */
            /*                     pthread_create(&thread, NULL, &run_command, */
            /*                         create_command(command, full_path)); */
            /*                     pthread_detach(thread); */
            /*                     free(full_path); */
            /*                     free(command); */
            /*                     exec_cmd = 1; */
            /*                 }else */
            /*                     err = ERR_PREV_COMMAND; */
            /*                 break; */
            case KEY_MOUSE  :
                            if(getmouse(&m_event) != ERR){
                                if(m_event.bstate == MOUSE6_PRESSED){
                                    c = 'h';
                                    c_updated = 1;
                                }else if(m_event.bstate == MOUSE7_PRESSED){
                                    c = 'l';
                                    c_updated = 1;
                                }else if(m_event.bstate & BUTTON4_PRESSED){
                                    c = 'u';
                                    c_updated = 1;
                                }else if(m_event.bstate & BUTTON5_PRESSED){
                                    c = 'd';
                                    c_updated = 1;
                                }else if(m_event.bstate & BUTTON2_PRESSED){
                                    c = 'a';
                                    c_updated = 1;
                                    while(getch() != KEY_MOUSE)
                                        ;
                                }else{
                                    index = (m_event.y > 0 &&
                                            m_event.y < df->num_items + 1 ?
                                            print_ind + (m_event.y - 1) :
                                            index);
                                    if(m_event.bstate & BUTTON1_PRESSED){
                                        if(!indices_init)
                                            indices = calloc(df->num_items + 1,
                                                    sizeof(int));
                                        toggle_index(indices, &indices_init, &n,
                                                index, df->num_items);
                                    }else if((m_event.bstate &
                                            BUTTON1_DOUBLE_CLICKED) ||
                                            m_event.bstate & BUTTON3_PRESSED){
                                        c = '\n';
                                        c_updated = 1;
                                    }
                                    while(getch() != KEY_MOUSE)
                                        ;
                                }
                            }else
                                err = ERR_MOUSE;
                            break;
            case KEY_RESIZE : 
                            getmaxyx(stdscr, max_y, max_x);
                            break;
            case -1         : 
                            df = get_files((path = get_start_path(argv,
                                      argc - 1)), 1);
                            break;
            default         :
                            err = ERR_INPUT;
        }
        if(info_txt != NULL && !exec_cmd && c != RESIZE){
            free(info_txt);
            info_txt = NULL;
        }
        update_display(df, path, index, print_ind, (indices_init ? 1 : 0),
                (indices_init ? indices : NULL));
        memset(window_name, '\0', WINDOW_NAME_LENGTH + 1);
        if(WINDOW_NAME_LENGTH > 12){
            strcpy(window_name, "cursbrowse - ");
            /* 'cursbrowse - ' + '$path'*/
            if(strlen(path) + 13 > WINDOW_NAME_LENGTH){
                for(i = WINDOW_NAME_LENGTH - 1, j = strlen(path) - 1; i > 15;
                        j--, i--)
                    window_name[i] = path[j];
                while(i > 12)
                    window_name[i--] = '.';
            }else{
                strcat(window_name, path);
            }
        }

        set_window_name(win, window_name);
        err = 0;
        exec_cmd = 0;
        if(!c_updated)
            c = getch();
        c_updated = 0;
    }while(c != 'q');
    cleanup_df(df, 1);
    if(matches_init)
        cleanup_fm(fm);
    cleanup_ils(ils_back);
    cleanup_ils(ils_forward);
    free(path);
    if(indices_init)
        free(indices);
    if(prev_command != NULL)
        free(prev_command);
    if(info_txt != NULL)
        free(info_txt);
    endwin();

    return 0;
}
