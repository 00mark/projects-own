#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xmu/WinUtil.h> 
#include <errno.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <mpd/client.h>
#include <alsa/asoundlib.h>
#include <alsa/control.h>
#include <dirent.h>

#define TIME_FORMAT "%H:%M %d-%m-%Y"
#define DEV_FILEPATH "/proc/net/dev"
#define STAT_FILEPATH "/proc/stat"
#define BATTERY_PRESENT_FILEPATH "/sys/class/power_supply/BAT0/present"
#define BATTERY_CAPACITY_FILEPATH "/sys/class/power_supply/BAT0/capacity"
#define BATTERY_STATUS_FILEPATH "/sys/class/power_supply/BAT0/status"
#define ROOT_FOLDER_PATH "/"
#define HOME_FOLDER_PATH "/home/mark/"
#define PROC_PATH "/proc/"
#define PROC_CMD_PATH "comm"
#define NETIF "wls1:"
#define MAX_IF_LENGTH 32
#define MAXSTR 4096
#define MAXIPLEN 20
#define EPSILON 0.00001
#define DEV_LINE_LENGTH 256
#define SEP_COLOR_FG "#eb7a32"
/* #define TIME_COLOR_FG "#000000" */
/* #define TIME_COLOR_BG "#c9b779" */
/* #define SOUND_COLOR_FG "#ffb87b" */
/* #define SOUND_COLOR_BG "#23303f" */
/* #define BAT_COLOR_FG "#111111" */
/* #define BAT_COLOR_BG "#be9058" */
/* #define MEM_COLOR_FG "#ff985b" */
/* #define MEM_COLOR_BG "#181b1f" */
/* #define CPU_COLOR_FG "#222222" */
/* #define CPU_COLOR_BG "#4f6a79" */
#define NET_COLOR_FG "#ccbbaa"
#define NET_COLOR_BG "#2a2020"
#define MID_COLOR_FG "#ae571f"
#define HIGH_COLOR_FG "#a02a2a"

#define TIME_COLOR_FG "#221100"
#define TIME_COLOR_BG "#674c4c"
#define SOUND_COLOR_FG "#221100"
#define SOUND_COLOR_BG "#564040"
#define BAT_COLOR_FG "#998877"
#define BAT_COLOR_BG "#483636"
#define MEM_COLOR_FG "#998877"
#define MEM_COLOR_BG "#3c2d2d"
#define CPU_COLOR_FG "#ccbbaa"
#define CPU_COLOR_BG "#322626"

static void die(char *reason);
static unsigned char *getdate(void);
static unsigned char *getram(void);
static unsigned char *gettemperature(void);
static unsigned char *getmpdstat(void);
static unsigned char *getvol(void);
static unsigned char *getlocalipaddr(void);
static unsigned char *getnetupdown(void);
static unsigned char *getcpuload(void);
static unsigned char *gethomerootfree(void);
static unsigned char *getcpuinfo(void);
static unsigned char *getnetinfo(void);
static unsigned char *getbatteryinfo(void);
static unsigned char *getsoundinfo(void);
static unsigned char *getmeminfo(void);
static unsigned char *getnetandcpuinfo(void);
static unsigned char *getseparator(void);
static unsigned char *getseparator_2(char *col1, char *col2);
static unsigned char *testbar(void);
static unsigned char *draw_1(int y, int x);
static unsigned char *draw_0(int y, int x);
static unsigned char *draw_k(int y, int x);
static unsigned char *draw_m(int y, int x);
static unsigned char *draw_bar(int y, int x, int length, int height,
        int num_intervals, double print_step, int start_growth, int growth_rate,
        double value, char *col_used, char *col_unused, char *col_bg);
static unsigned char* itos(int d);
static unsigned char* smprintf(unsigned char *fmt, ...);
static long long int* getnetupdownvals(void);
static long long int* getcpuloadvals(void);
static unsigned char*get_full_path(unsigned char *p, ...);
static int isnumber(unsigned char *str);
static int is_lower_letter(unsigned char c);
static unsigned char *cmdcheck(unsigned char *cmd, const unsigned char *dptr);
static unsigned char *upper(unsigned char *str);
static void XSetRoot(unsigned char *name);
static unsigned char*(*functab[])(void)={
    getnetandcpuinfo, getmeminfo, getbatteryinfo, getsoundinfo, getdate
};
static char *colors[8] = {CPU_COLOR_BG, MEM_COLOR_BG, BAT_COLOR_BG,
    SOUND_COLOR_BG, TIME_COLOR_BG};

int main(void){
    Display *display;
    Window root;
    unsigned char status[MAXSTR];
    unsigned char *sta, *cur, *sep;
    int left, i, ret, err;

    display=XOpenDisplay(NULL);
    if(display == NULL){
        fprintf(stderr, "ERROR: could not open display\n");
        exit(1);
    }
    root = XRootWindow(display ,DefaultScreen(display));
    for(;;){
        left = sizeof(status) - 1;
        sta = status;
        for(i = 0; i < sizeof(functab) / sizeof(functab[0]); ++i){
            cur = functab[i]();
            if(i == sizeof(functab) / sizeof(functab[0]) - 1){
                ret = snprintf(sta, left, "%s", cur);
            }else{
                sep = getseparator_2(colors[i], colors[i+1]);
                ret = snprintf(sta, left, "%s%s", cur, sep);
                free(sep);
            }
            sta += ret;
            left -= ret;
            free(cur);
            if(sta >= (status + MAXSTR))
                break;
        }
        err = XStoreName(display, root, status);
        if(err != 1)
            fprintf(stderr, "%d\n", err);
        XSync(display, 0);
    }
    XCloseDisplay(display);
    return 0;
}

static void die(char *reason){
    printf("Error: %s\n", reason);
    exit(0);
}

static unsigned char *getseparator(void){
    unsigned char *ret;

    ret = NULL;
    ret = malloc(256);
    if(!ret)
        die("cannot malloc");

    snprintf(ret, 256, " ^c%s^^r0,7,1,1^^r1,6,1,3^^r2,5,1,5^^r3,4,1,3^"
            "^r3,8,1,3^^r4,3,1,3^^r4,9,1,3^^r5,2,1,3^^r5,10,1,3^^r6,1,1,3^"
            "^r6,11,1,3^^r7,0,1,3^^r7,12,1,3^^r8,0,1,2^^r8,13,1,2^^r9,0,1,1^"
            "^r9,14,1,1^^f10^ ", SEP_COLOR_FG);

    return ret;
}

static unsigned char *getseparator_2(char *col1, char *col2){
    unsigned char *ret;

    ret = NULL;
    ret = malloc(512);
    if(!ret)
        die("cannot malloc");

    snprintf(ret, 512, "^c%s^^r0,0,7,1^^r0,1,6,1^^r0,2,5,1^^r0,3,4,1^"
            "^r0,4,3,1^^r0,5,2,1^^r0,6,1,1^^r0,8,1,1^^r0,9,2,1^^r0,10,3,1^"
            "^r0,11,4,1^^r0,12,5,1^^r0,13,6,1^^r0,14,7,1^"
            "^c%s^^r0,7,1,1^^r1,6,1,3^^r2,5,1,5^^r3,4,1,7^^r4,3,1,9^^r5,2,1,11^"
            "^r6,1,1,13^^r7,0,3,15^^f10^", col1, col2);

    return ret;
}

static unsigned char *testbar(void){
    unsigned char *ret, *one, *one2, *one3, *one4, *zero, *zero2, *zero3,
                  *zero4, *zero5, *k, *m, *m2, *m3, *bar, *bar2;
    long long int *pre_updown, *post_updown;
    double down, up;

    ret = NULL;
    ret = malloc(4092);
    if(!ret)
        die("cannot malloc");

    pre_updown = getnetupdownvals();
    sleep(1);
    post_updown = getnetupdownvals();
    down = (post_updown[1] - pre_updown[1]);
    up = (post_updown[0] - pre_updown[0]);
    snprintf(ret, 4092, "%s%s^c#cabe9b^%s%s%s%s%s%s%s%s%s%s%s%s%s"
            "^c%s^^r50,0,1,15^^r100,0,1,15^^r150,0,1,15^^f425^",
            (bar = draw_bar(7, 0, 200, 8, 4, 5, 10000, 10,
                            down, SEP_COLOR_FG, "#999999", "#ffffff")), (bar2 = draw_bar(7, 225,
                                200, 8, 4, 5, 1000, 10, up, "#49a377", "#999999", "#000000")),
            (one = draw_1(1, 33)), (zero = draw_0(1, 37)), (zero2 = draw_0(1,
                    41)), (k = draw_k(1, 45)) , (one2 = draw_1(1, 90)) , (m =
                    draw_m(1, 94)), (one3 = draw_1(1, 136)), (zero3 = draw_0(1,
                        140)) , (m2 = draw_m(1, 144)), (one4 = draw_1(1, 182)),
            (zero4 = draw_0(1, 186)), (zero5 = draw_0(1, 190)), (m3 = draw_m(1,
                    194)), "#888888");
    free(bar);
    free(bar2);
    free(one);
    free(one2);
    free(one3);
    free(one4);
    free(zero);
    free(zero2);
    free(zero3);
    free(zero4);
    free(zero5);
    free(k);
    free(m);
    free(m2);
    free(pre_updown);
    free(post_updown);

    return ret;
}

static unsigned char *draw_1(int y, int x){
    unsigned char *ret;

    ret = NULL;
    ret = malloc(64);
    if(!ret)
        die("cannot malloc");

    snprintf(ret, 64, "^r%d,%d,1,1^^r%d,%d,1,1^^r%d,%d,1,5^", x, y + 2, x + 1,
            y + 1, x + 2, y);

    return ret;
}

static unsigned char *draw_0(int y, int x){
    unsigned char *ret;

    ret = NULL;
    ret = malloc(64);
    if(!ret)
        die("cannot malloc");

    snprintf(ret, 64, "^r%d,%d,1,5^^r%d,%d,1,1^^r%d,%d,1,1^"
            "^r%d,%d,1,5^", x, y, x + 1, y + 4, x + 1, y, x + 2, y);

    return ret;
}

static unsigned char *draw_k(int y, int x){
    unsigned char *ret;

    ret = NULL;
    ret = malloc(128);
    if(!ret)
        die("cannot malloc");

    snprintf(ret, 128, "^r%d,%d,1,5^^r%d,%d,1,1^^r%d,%d,1,1^^r%d,%d,1,1^"
            "^r%d,%d,1,1^^r%d,%d,1,1^", x, y, x + 1, y + 2, x + 2, y + 3,
            x + 2, y + 1, x + 3, y + 4, x + 3, y);

    return ret;
}

static unsigned char *draw_m(int y, int x){
    unsigned char *ret;

    ret = NULL;
    ret = malloc(128);
    if(!ret)
        die("cannot malloc");

    snprintf(ret, 128, "^r%d,%d,1,5^^r%d,%d,1,1^^r%d,%d,1,1^^r%d,%d,1,1^"
            "^r%d,%d,1,5^", x, y, x + 1, y + 1, x + 2, y + 2, x + 3, y + 1,
            x + 4, y);

    return ret;
}

static unsigned char *draw_bar(int y, int x, int length, int height,
        int num_intervals, double print_step, int start_growth, int growth_rate,
        double value, char *col_used, char *col_unused, char *col_bg){
    unsigned char *ret; 
    int cur_step, j;
    double i, cur_val, used_len;

    ret = NULL;
    ret = malloc(256);
    if(!ret)
        die("cannot malloc");

    cur_val = used_len = 0.0;
    cur_step = start_growth;
    while(used_len < length && cur_val < value){
        for(i = 0.0; used_len < length && cur_val < value &&
                i < length / num_intervals; i += print_step){
            cur_val += cur_step; 
            used_len += print_step;
        }
        cur_step *= growth_rate;
    }
    j = snprintf(ret, 256, "^c%s^^r%d,%d,%d,%d^^c%s^^r%d,%d,%d,%d^", col_used,
            x, y, (int)used_len, height, col_unused, x + (int)used_len, y,
            length - ((int)used_len), height);
    if(y || y + height < 15){
        j += snprintf(&(ret[j]), 256 - j, "^c%s^", col_bg);
        if(y)
            j += snprintf(&(ret[j]), 256 - j, "^r%d,%d,%d,%d^", x, 0, length,
                    y);
        if(y + height < 15)
            j += snprintf(&(ret[j]), 256 - j, "^r%d,%d,%d,%d^", x, y + height,
                    length, 15 - (y + height));
    }

    return ret;
}

static unsigned char *getdate(void){
    int i;
    unsigned char *ret;
    time_t now = time(0);

    ret = NULL;
    ret = malloc(64);
    if(!ret)
        die("cannot malloc");

    i = snprintf(ret, 64, "^c%s^^g%s^", TIME_COLOR_FG, TIME_COLOR_BG);
    strftime(&(ret[i]), 64 - i, TIME_FORMAT, localtime(&now));

    return ret;
}

static unsigned char *getram(void){
    unsigned char *ret;
    struct sysinfo *s;

    ret = NULL;
    ret = malloc(8);
    if(!ret)
        die("cannot malloc");

    s = NULL;
    s = malloc(sizeof(*s));
    if(!s)
        die("cannot malloc");

    sysinfo(s);
    snprintf(ret, 8, "%5.1f", (double)(s->totalram - s->freeram) / s->totalram *
            100);
    free(s);

    return ret;
}

static unsigned char *gettemperature(){
    FILE *fp;
    int temp;
    unsigned char *ret;

    ret = NULL;
    ret = malloc(8);
    if(!ret)
        die("cannot malloc");

    fp = fopen("/sys/class/hwmon/hwmon1/temp1_input", "r");
    if(fp == NULL){
        strcpy(ret, "t err");
        return ret;
    }
    fscanf(fp, "%d", &temp);
    fclose(fp);
    snprintf(ret, 8, "%d", temp / 1000);

    return ret;
}

static unsigned char *gethomerootfree(){
    unsigned char *ret;
    struct statvfs *rootbuf, *homebuf; 
    int gb;

    ret = NULL;
    ret = malloc(32);
    if(!ret)
        die("cannot malloc");

    gb = 1024 * 1024 * 1024;
    rootbuf = NULL;
    rootbuf = malloc(sizeof(*rootbuf));
    if(!rootbuf)
        die("cannot malloc");

    homebuf = NULL;
    homebuf = malloc(sizeof(*homebuf));
    if(!homebuf)
        die("cannot malloc");

    statvfs(ROOT_FOLDER_PATH, rootbuf);
    statvfs(HOME_FOLDER_PATH, homebuf);
    snprintf(ret, 32, "/ %.1f GiB|⌂ %.1f GiB", (double)(rootbuf->f_bavail
                * rootbuf->f_frsize) / gb, (double)(homebuf->f_bavail *
                    homebuf->f_frsize) / gb);
    free(rootbuf);
    free(homebuf);

    return ret;
}

static unsigned char *getmeminfo(){
    unsigned char *ret, *ram, *hrfree;
    int i;

    ram = getram();
    hrfree = gethomerootfree();
    ret = NULL;
    ret = malloc(128);
    if(!ret)
        die("cannot malloc");

    i = snprintf(ret, 128, "^c%s^^g%s^", MEM_COLOR_FG, MEM_COLOR_BG);
    if(atoi(ram) > 80)
        i += snprintf(&(ret[i]), 128 - i, "^c%s^%s%%^c%s^ - ", HIGH_COLOR_FG, ram,
                MEM_COLOR_FG);
    else if(atoi(ram) > 60)
        i += snprintf(&(ret[i]), 128 - i, "^c%s^%s%%^c%s^ - ", MID_COLOR_FG, ram,
                MEM_COLOR_FG);
    else
        i += snprintf(&(ret[i]), 128 - i, "%s%% - ", ram);
    snprintf(&(ret[i]), 128 - i, "%s ^d^", hrfree);
    free(ram);
    free(hrfree);

    return ret;
}

static unsigned char *getmpdstat(){
    struct mpd_song * song = NULL;
    unsigned char * title = NULL;
    unsigned char * artist = NULL;
    unsigned char * retstr = NULL;
    int elapsed = 0, total = 0;
    struct mpd_connection * conn;

    if (!(conn = mpd_connection_new("localhost", 0, 30000)) ||
            mpd_connection_get_error(conn)){
        return smprintf("");
    }
    mpd_command_list_begin(conn, true);
    mpd_send_status(conn);
    mpd_send_current_song(conn);
    mpd_command_list_end(conn);
    struct mpd_status* theStatus = mpd_recv_status(conn);
    if((theStatus) && (mpd_status_get_state(theStatus) == MPD_STATE_PLAY)){
        mpd_response_next(conn);
        song = mpd_recv_song(conn);
        title = smprintf("%s",mpd_song_get_tag(song, MPD_TAG_TITLE, 0));
        if(strlen(title) > 15)
            strcpy(&(title[12]), "...");
        artist = smprintf("%s",mpd_song_get_tag(song, MPD_TAG_ARTIST, 0));
        if(strlen(artist) > 15)
            strcpy(&(artist[12]), "...");

        elapsed = mpd_status_get_elapsed_time(theStatus);
        total = mpd_status_get_total_time(theStatus);
        mpd_song_free(song);
        retstr = smprintf("%.2d:%.2d/%.2d:%.2d %s - %s",
                elapsed/60, elapsed%60,
                total/60, total%60,
                artist, title);
        free((unsigned char*)title);
        free((unsigned char*)artist);
    }
    else
        retstr = smprintf("");
    mpd_status_free(theStatus);
    mpd_response_finish(conn);
    mpd_connection_free(conn);

    return retstr;
}

static unsigned char *getvol(void){
    int vol;
    unsigned char *ret;

    ret = NULL;
    ret = malloc(4);
    if(!ret)
        die("cannot malloc");

    snd_hctl_t *hctl;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_value_t *control;
    snd_hctl_open(&hctl, "hw:0", 0);
    snd_hctl_load(hctl);
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
    snd_ctl_elem_id_set_name(id, "Master Playback Volume");
    snd_hctl_elem_t *elem = snd_hctl_find_elem(hctl, id);
    snd_ctl_elem_value_alloca(&control);
    snd_ctl_elem_value_set_id(control, id);
    snd_hctl_elem_read(elem, control);
    vol = (int)snd_ctl_elem_value_get_integer(control,0);
    snd_hctl_close(hctl);
    sprintf(ret, "%3d", vol);

    return ret;
}

static unsigned char *getsoundinfo(void){
    unsigned char *ret, *vol, *mpdstat, *bar;
    int i;

    vol = getvol();
    mpdstat = getmpdstat();
    ret = NULL;
    ret = malloc(256);
    if(!ret)
        die("cannot malloc");

    i = snprintf(ret, 256, "^c%s^^g%s^", SOUND_COLOR_FG,
            SOUND_COLOR_BG);
    i += snprintf(&(ret[i]), 256 - i, "%s^f50^^c%s^ - ", (bar = draw_bar(4, 0,
                    50, 8, 1, 0.5, 1, 1, atoi(vol), (atoi(vol) > 65 ?
                        HIGH_COLOR_FG : atoi(vol) > 55 ? MID_COLOR_FG :
                        SOUND_COLOR_FG), "#333333", SOUND_COLOR_BG)),
            SOUND_COLOR_FG);
    snprintf(&(ret[i]), 256- i, "%s ^d^", mpdstat);
    free(mpdstat);
    free(vol);
    free(bar);

    return ret;
}

static unsigned char *getlocalipaddr(void){
    FILE *addrp;
    unsigned char *ret;
    char c;
    int first, found, i, j;

    ret = NULL;
    ret = malloc(MAXIPLEN);
    if(!ret)
        die("cannot malloc");

    addrp = fopen("/proc/net/fib_trie", "r");
    if(addrp == NULL){
        strcpy(ret, "NO IP");
        return ret;
    }
    first = 1;
    found = 0;
    while((c = fgetc(addrp)) != EOF){
        if(first && c == '1'){
            for(i = 1; i < MAXIPLEN; i++)
                ret[i] = '\0';
            ret[0] = c;
            for(i = 1; i < 7; i++)
                ret[i] = fgetc(addrp);
            ret[i] = '\0';
            if(!strcmp(ret, "192.168")){
                fscanf(addrp, "%s", &(ret[i]));
                j = strlen(ret);
                while(j >= 6 && ret[j] != '/')
                    j--;
                if(j >= 6)
                    first = 0;
                else{    
                    j = strlen(ret);
                    if((ret[j-1] == '0' && ret[j-2] == '.') ||
                            ret[j-3] == '2' && ret[j-2] == '2' && ret[j-1] == 
                            '5')
                        first = 0;
                    else{
                        found = 1;
                        break;
                    }
                }
            }else
                first = 0;
        }else if(c == '\n')
            first = 1;
        else if(c != ' ' && c != '-' && c != '+' && c != '|')
            first = 0;
    }
    fclose(addrp);
    if(!found)
        strcpy(ret, "NO IP");

    return ret;
}

static long long int *getnetupdownvals(void){
    FILE *devfp;
    unsigned char tmp[DEV_LINE_LENGTH], ifname[MAX_IF_LENGTH];
    long long int down_bytes, down_packets, down_errs, down_drop, down_fifo,
         down_frame, down_compressed, down_multicast, up_bytes, up_packets,
         up_errs, up_drop, up_frame, up_fifo, up_compressed, up_multicast;
    long long int *ret;

    ret = NULL;
    ret = malloc(sizeof(long long int) * 2);
    if(!ret)
        die("cannot malloc");

    devfp = fopen(DEV_FILEPATH, "r");
    if(devfp == NULL){
        ret[0] = ret[1] = -1;
        return ret;
    }
    fgets(tmp, DEV_LINE_LENGTH, devfp);
    do{
        fgets(tmp, DEV_LINE_LENGTH, devfp);
        if(fscanf(devfp, "%s:", ifname) == EOF){
            fclose(devfp);
            ret[0] = ret[1] = -2;
            return ret;
        }
    }while(strcmp(ifname, NETIF));
    fscanf(devfp, "%lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld "
            "%lld %lld %lld %lld %lld", &down_bytes, &down_packets,
            &down_errs, &down_drop, &down_fifo, &down_frame,
            &down_compressed, &down_multicast, &up_bytes, &up_packets,
            &up_errs, &up_drop, &up_fifo, &up_frame, &up_compressed,
            &up_multicast);
    fclose(devfp); 
    ret[0] = up_bytes;
    ret[1] = down_bytes;

    return ret;
}

static unsigned char *getnetupdown(void){
    unsigned char *ret;
    long long int *pre_updown, *post_updown;

    ret = NULL;
    ret = malloc(64);
    if(!ret)
        die("cannot malloc");

    pre_updown = getnetupdownvals();
    usleep(500 * 1000);
    post_updown = getnetupdownvals();
    sprintf(ret, "UP: %5ld KiB, DOWN: %5ld KiB", (post_updown[0] - pre_updown[
                0]) * 2 / 1024, (post_updown[1] - pre_updown[1]) * 2 / 1024);
    free(pre_updown);
    free(post_updown);

    return ret;
}

static unsigned char *getnetinfo(void){
    unsigned char prev_c, *buf, *ret, *vpn, *updown, *addr, *cmd;
    int updown_len;

    cmd = NULL;
    cmd = malloc(8);
    if(!cmd)
        die("cannot malloc");

    strcpy(cmd, "openvpn");
    vpn = cmdcheck(cmd, "VPN");
    addr = getlocalipaddr();
    updown = getnetupdown();
    for(updown_len = 0; updown_len < 64; updown_len++){
        if(updown[updown_len] == '\0')
            break;
    }
    ret = NULL;
    ret = malloc(updown_len + strlen(vpn) + strlen(addr) + 25);
    if(!ret)
        die("cannot malloc");

    *strcpy(ret, "NET: [");
    memcpy(&(ret[7]), updown, updown_len);
    sprintf(&(ret[7 + updown_len]), "] [%s] [%s]", addr, vpn);
    free(vpn);
    free(updown);
    free(addr);

    return ret;
}

static long long int *getcpuloadvals(void){
    FILE *statfp;
    long long int *ret;
    unsigned char buf[4];

    ret = NULL;
    ret = malloc(sizeof(long long int) * 4);
    if(!ret)
        die("cannot malloc");

    statfp = fopen(STAT_FILEPATH, "r");
    if(statfp == NULL){
        ret[0] = ret[1] = ret[2] = ret[3] = -1;
        return ret;
    }
    fscanf(statfp, "%s %lld %lld %lld %lld", buf, &(ret[0]), &(ret[1]), 
            &(ret[2]), &(ret[3]));
    fclose(statfp);

    return ret;
}

static unsigned char* getcpuload(void){
    unsigned char *ret;
    long long int *pre_load, *post_load;

    ret = NULL;
    ret = malloc(8);
    if(!ret)
        die("cannot malloc");

    pre_load = getcpuloadvals();
    usleep(500*1000);
    post_load = getcpuloadvals();
    sprintf(ret, "%3.0f%%", 100 * ((double)((post_load[0] + post_load[1] +
                        post_load[2]) - (pre_load[0] + pre_load[1] + pre_load[2])) /
                ((post_load[0] + post_load[1] + post_load[2] + post_load[3]) -
                 (pre_load[0] + pre_load[1] + pre_load[2] + pre_load[3]))));
    free(pre_load);
    free(post_load);

    return ret;
}

static unsigned char *getcpuinfo(void){
    unsigned char *ret, *load, *temp;

    temp = gettemperature();
    load = getcpuload();
    ret = NULL;
    ret = malloc(strlen(temp) + strlen(load) + 19);
    if(!ret)
        die("cannot malloc");

    sprintf(ret, "CPU: [%s] [%s]", load, temp); 
    free(load);
    free(temp);

    return ret;
}

static unsigned char *getnetandcpuinfo(void){
    unsigned char *ret, *vpn, *addr, *temp, *sep, *cmd;
    long long int *pre_updown, *post_updown, *pre_load, *post_load;
    int i, up, down;
    double load;

    cmd = NULL;
    cmd = malloc(8);
    if(!cmd)
        die("cannot malloc");

    strcpy(cmd, "openvpn");
    ret = NULL;
    ret = malloc(1024);
    if(!ret)
        die("cannot malloc");

    pre_load = getcpuloadvals();
    pre_updown = getnetupdownvals();
    sleep(1);
    post_updown = getnetupdownvals();
    post_load = getcpuloadvals();
    vpn = cmdcheck(cmd, "VPN");
    free(cmd);
    addr = getlocalipaddr();
    temp = gettemperature();
    sep = getseparator_2(NET_COLOR_BG, CPU_COLOR_BG);
    up = (post_updown[0] - pre_updown[0]) / 1024;
    down = (post_updown[1] - pre_updown[1]) / 1024;
    load = 100 * ((double)((post_load[0] + post_load[1] + post_load[2]) -
                (pre_load[0] + pre_load[1] + pre_load[2])) / ((post_load[0] +
                        post_load[1] + post_load[2] + post_load[3]) - (pre_load[0] +
                            pre_load[1] + pre_load[2] + pre_load[3])));
    i = snprintf(ret, 1024, "^c%s^^g%s^ ↑ ", NET_COLOR_FG,
            NET_COLOR_BG);
    if(up > 500)
        i += snprintf(&(ret[i]), 1024 - i, "^c%s^%5d KiB^c%s^", HIGH_COLOR_FG,
                up, NET_COLOR_FG);
    else if(up > 200)
        i += snprintf(&(ret[i]), 1024 - i, "^c%s^%5d KiB^c%s^", MID_COLOR_FG,
                up, NET_COLOR_FG);
    else
        i += snprintf(&(ret[i]), 1024 - i, "%5d KiB", up);
    if(down > 5000)
        i += snprintf(&(ret[i]), 1024 - i, "|↓ ^c%s^%5d KiB^c%s^ - ", 
                HIGH_COLOR_FG, down, NET_COLOR_FG);
    else if(down > 1000)
        i += snprintf(&(ret[i]), 1024 - i, "|↓ ^c%s^%5d KiB^c%s^ - ",
                MID_COLOR_FG, down, NET_COLOR_FG);
    else
        i += snprintf(&(ret[i]), 1024 - i, "|↓ %5d KiB - ", down);
    if(!strcmp(addr, "NO IP"))
        i += snprintf(&(ret[i]), 1024 - i, "^c%s^%s^c%s^ - ", HIGH_COLOR_FG,
                addr, NET_COLOR_FG);
    else
        i += snprintf(&(ret[i]), 1024 - i, "%s - ", addr);
    if(strcmp(vpn, "VPN"))
        i += snprintf(&(ret[i]), 1024 - i, "^c%s^%s^c%s^ %s^c%s^^g%s^",
                HIGH_COLOR_FG, vpn, NET_COLOR_FG, sep, CPU_COLOR_FG,
                CPU_COLOR_BG);
    else
        i += snprintf(&(ret[i]), 1024 - i, "%s %s^c%s^^g%s^", vpn, sep,
                CPU_COLOR_FG, CPU_COLOR_BG);
    if(load > 80)
        i += snprintf(&(ret[i]), 1024 - i, "^c%s^%3.0f%%^c%s^ - ",
                HIGH_COLOR_FG, load, CPU_COLOR_FG);
    else if(load > 50)
        i += snprintf(&(ret[i]), 1024 - i, "^c%s^%3.0f%%^c%s^ - ", MID_COLOR_FG,
                load, CPU_COLOR_FG);
    else
        i += snprintf(&(ret[i]), 1024 - i, "%3.0f%% - ", load);
    if(atoi(temp) > 80)
        snprintf(&(ret[i]), 1024 - i, "^c%s^%s°C^c%s^ ^d^", HIGH_COLOR_FG,
                temp, CPU_COLOR_FG);
    else if(atoi(temp) > 60)
        snprintf(&(ret[i]), 1024 - i, "^c%s^%s°C^c%s^ ^d^", MID_COLOR_FG, temp,
                CPU_COLOR_FG);
    else
        snprintf(&(ret[i]), 1024 - i, "%s°C ^d^", temp);
    free(vpn);
    free(addr);
    free(temp);
    free(pre_updown);
    free(post_updown);
    free(pre_load);
    free(post_load);
    free(sep);

    return ret;
}

static unsigned char *getbatteryinfo(void){
    FILE *batfp, *tmp1fp, *tmp2fp;
    unsigned char *ret, *bar, status[16];
    int present, capacity, i;

    batfp = tmp1fp = tmp2fp = NULL;
    ret = NULL;
    ret = malloc(256);
    if(!ret)
        die("cannot malloc");

    if((batfp = fopen(BATTERY_PRESENT_FILEPATH, "r")) == NULL ||
            (tmp1fp = fopen(BATTERY_CAPACITY_FILEPATH, "r")) == NULL ||
            (tmp2fp = fopen(BATTERY_STATUS_FILEPATH, "r")) == NULL){
        snprintf(ret, 256, "^c%s^^g%s^^c%s^bat err^c%s^ ^d^",
                BAT_COLOR_FG, BAT_COLOR_BG, HIGH_COLOR_FG, BAT_COLOR_FG);
        if(batfp != NULL)
            fclose(batfp);
        if(tmp1fp != NULL)
            fclose(tmp1fp);
        if(tmp2fp != NULL)
            fclose(tmp2fp);
        return ret;
    }
    fclose(tmp1fp);
    fclose(tmp2fp);
    fscanf(batfp, "%d", &present);
    fclose(batfp);
    if(present){
        batfp = fopen(BATTERY_CAPACITY_FILEPATH, "r");
        fscanf(batfp, "%d", &capacity);
        fclose(batfp);
        batfp = fopen(BATTERY_STATUS_FILEPATH, "r");
        fscanf(batfp, "%s", status);
        fclose(batfp);
    }else{
        snprintf(ret, 256, "^c%s^^g%s^^c%s^not present^c%s^ ^d^",
                BAT_COLOR_FG, BAT_COLOR_BG, HIGH_COLOR_FG, BAT_COLOR_FG);
        return ret;
    }
    i = snprintf(ret, 256, "^c%s^^g%s^", BAT_COLOR_FG, BAT_COLOR_BG);
    i += snprintf(&(ret[i]), 256- i, "%s^f50^^c%s^ - ", (bar = draw_bar(4, 0,
                    50, 8, 1, 0.5, 1, 1, capacity, capacity < 10 ?
                    HIGH_COLOR_FG : capacity < 30 ? MID_COLOR_FG : BAT_COLOR_FG,
                    "#333333", BAT_COLOR_BG)),
            BAT_COLOR_FG);
    if(strcmp(status, "Discharging"))
        snprintf(&(ret[i]), 256- i, "%c ^d^", '+');
    else
        snprintf(&(ret[i]), 256- i, "^c%s^%c^c%s^ ^d^", HIGH_COLOR_FG, '-',
                BAT_COLOR_FG);
    free(bar);

    return ret;
}

static void XSetRoot(unsigned char *name){
    Display *display;

    if((display = XOpenDisplay(NULL)) == NULL){
        fprintf(stderr, "[barM] cannot open display!\n");
        exit(1);
    }

    XStoreName(display, DefaultRootWindow(display), name);
    XSync(display, 0);

    XCloseDisplay(display);
}

static unsigned char *itos(int d){
    int len, i, tmp;
    unsigned char *ret;

    if(d > 0)
        for(len = 2, tmp = d; tmp > 0; tmp /= 10, len++);
    else
        len = 3;
    ret = NULL;
    ret = malloc(len);
    if(!ret)
        die("cannot malloc");

    ret[(i = len - 1)] = '\0';
    i--;
    ret[i--] = '\0';
    if(d <= 0)
        ret[0] = '0';
    else{
        while(d > 0){
            ret[i--] = (d % 10) + 48;  
            d /= 10;
        }
    }

    return ret;
}

static unsigned char *smprintf(unsigned char *fmt, ...){
    va_list fmtargs;
    unsigned char *ret;
    int len;

    va_start(fmtargs, fmt);
    len = vsnprintf(NULL, 0, fmt, fmtargs);
    va_end(fmtargs);

    ret = NULL;
    ret = malloc(++len);

    if (ret == NULL) {
        perror("malloc");
        exit(1);
    }

    va_start(fmtargs, fmt);
    vsnprintf(ret, len, fmt, fmtargs);
    va_end(fmtargs);

    return ret;
}

static unsigned char *cmdcheck(unsigned char *cmd, const unsigned char *dptr){
    FILE *tmp;
    DIR *d;
    struct dirent *dir;    
    unsigned char *ret, *path;
    char *comm, tmp_cmd[strlen(cmd) + 2];
    size_t len = 0;

    strcpy(tmp_cmd, cmd);
    strcat(tmp_cmd, "\n");
    comm = ret = path = NULL;
    ret = malloc(strlen(dptr) + 5);

    if(ret == NULL)
        return NULL;
    if(!(d = opendir(PROC_PATH))){
        strcpy(ret, dptr);
        strcat(ret, " ERR");
        return ret;
    }
    while((dir = readdir(d)) != NULL){
        if(isnumber(dir->d_name)){
            path = get_full_path(PROC_PATH, dir->d_name, PROC_CMD_PATH,
                    (char *)(0));
            if(!(tmp = fopen(path, "r"))){
                free(path);
                continue;
            }
            getline(&comm, &len, tmp);
            len = 0;
            fclose(tmp); 
            free(path);
            if(comm != NULL){
                if(!strcmp(comm, tmp_cmd)){
                    closedir(d);
                    free(comm);
                    strcpy(ret, dptr);
                    return ret;
                }
                free(comm);
                comm = NULL;
            }
        }
    }
    closedir(d);
    strcpy(ret, "NO ");
    strcat(ret, dptr);
    return ret;
}

static unsigned char *get_full_path(unsigned char *p, ...){
    int len, num, cur;
    unsigned char *path, *tmp;
    va_list ap;

    path = NULL;
    tmp = p;
    len = num = 0;
    va_start(ap, p);
    while(tmp != NULL){
        num++;
        len += (tmp[strlen(tmp) - 1] != '/' ? strlen(tmp) + 1 : strlen(tmp));
        tmp = va_arg(ap, unsigned char *);
    }
    len++;
    va_end(ap);

    path = malloc(len);

    if(path == NULL)
        return NULL;
    memset(path, '\0', len);
    va_start(ap, p);
    cur = 0;
    while(p != NULL){
        cur++;
        strcat(path, p);
        if(cur != num && p[strlen(p) - 1] != '/')
            strcat(path, "/");
        p = va_arg(ap, unsigned char *);
    }
    va_end(ap);

    return path;
}

static int isnumber(unsigned char *str){
    int i = 0;

    while(str[i] >= '0' && str[i] <= '9')
        i++;

    return i > 0 && str[i] == '\0';
}

static int is_lower_letter(unsigned char c){
    return c >= 'a' && c <= 'z';
}

static unsigned char *upper(unsigned char* str){
    int i;

    i = 0;
    while(str[i] != '\0'){
        if(is_lower_letter(str[i]))
            str[i] -= ('a' - 'A');
        i++;
    }

    return str;
}
