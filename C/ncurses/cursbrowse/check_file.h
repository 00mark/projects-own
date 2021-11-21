#include <stdio.h>

#ifndef CHECK_FILE
#define CHECK_FILE

#define SAMPLE_SIZE 1000
#define ERR_FILE -1
#define UNKNOWN 0 
#define TEXT 1
#define ELF 2
#define EXE 3
#define ISO 4
#define MKV_WEBM 5
#define AVI 6
#define GIF 7
#define MPG 8
#define FLAC 9
#define WAV 10
#define MP3 11
#define PDF 12
#define JPG 13
#define PNG 14
#define BMP 15
#define TAR_GZ 16
#define ZIP 17 
#define RAR 18
#define BZ2 19
#define LZ 20
#define ZLIB 21

int check(char *path);
int check_magic(FILE *fp);
int is_txt(FILE *fp);
int is_elf(char *bytes, int len);
int is_exe(char *bytes, int len);
int is_iso(char *bytes, int len);
int is_mkv(char *bytes, int len);
int is_avi(char *bytes, int len);
int is_gif(char *bytes, int len);
int is_mpg(char *bytes, int len);
int is_flac(char *bytes, int len);
int is_wav(char *bytes, int len);
int is_mp3(char *bytes, int len);
int is_pdf(char *bytes, int len);
int is_jpg(char *bytes, int len);
int is_png(char *bytes, int len);
int is_bmp(char *bytes, int len);
int is_gz(char *bytes, int len);
int is_zip(char *bytes, int len);
int is_rar(char *bytes, int len);
int is_bz2(char *bytes, int len);
int is_lz(char *bytes, int len);
int is_zlib(char *bytes, int len);

#endif
