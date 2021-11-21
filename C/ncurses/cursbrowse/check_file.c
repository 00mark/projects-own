#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cursbrowse.h"
#include "check_file.h"
#include "util.h"

int check(char *path)
{
    FILE *fp;
    int ret;

    if(!(fp = fopen(path, "r"))) 
        return ERR_FILE;
    if((ret = check_magic(fp)))
        return ret;
    rewind(fp);
    ret = is_txt(fp);
    fclose(fp);

    return ret;
}

int is_txt(FILE *fp)
{
    int i, c;

    for(i = 0; i < SAMPLE_SIZE && (c = fgetc(fp)) != EOF; i++){
        if(c > 0x7f)
            return UNKNOWN;
    }
    return TEXT;
}

int check_magic(FILE *fp)
{
    char *bytes;
    int i, len, ret;

    bytes = malloc(20);
    for(i = 0; i < 20; i++)
        bytes[i] = END;
    fscanf(fp, "%19c", bytes);
    for(i = len = 0; i < 20 && bytes[i] != END; len++, i++)
        ;
    if(is_elf(bytes, len))
        ret = ELF;
    else if(is_exe(bytes, len))
        ret = EXE;
    else if(is_iso(bytes, len))
        ret = ISO;
    else if(is_mkv(bytes, len))
        ret = MKV_WEBM;
    else if(is_avi(bytes, len))
        ret = AVI;
    else if(is_gif(bytes, len))
        ret = GIF;
    else if(is_mpg(bytes, len))
        ret = MPG;
    else if(is_flac(bytes, len))
        ret = FLAC;
    else if(is_wav(bytes, len))
        ret = WAV;
    else if(is_mp3(bytes, len))
        ret = MP3;
    else if(is_pdf(bytes, len))
        ret = PDF;
    else if(is_jpg(bytes, len))
        ret = JPG;
    else if(is_png(bytes, len))
        ret = PNG;
    else if(is_bmp(bytes, len))
        ret = BMP;
    else if(is_gz(bytes, len))
        ret = TAR_GZ;
    else if(is_zip(bytes, len))
        ret = ZIP;
    else if(is_rar(bytes, len))
        ret = RAR;
    else if(is_bz2(bytes, len))
        ret = BZ2;
    else if(is_lz(bytes, len))
        ret = LZ;
    else if(is_zlib(bytes, len))
        ret = ZLIB;
    else
        ret = UNKNOWN;
    free(bytes);

    return ret;
}

int is_elf(char *bytes, int len)
{
    if(len < 4)
        return 0;
    return bytes[0] == 0x7f && bytes[1] == 0x45 && bytes[2] == 0x4c &&
        bytes[3] == 0x46;
}

int is_exe(char *bytes, int len)
{
    if(len < 2)
        return 0;
    return bytes[0] == 0x4d && bytes[1] == 0x5a;
}

int is_iso(char *bytes, int len)
{
    if(len < 5)
        return 0;
    return bytes[0] == 0x43 && bytes[1] == 0x44 && bytes[2] == 0x30 &&
        bytes[3] == 0x30 && bytes[4] == 0x31;
}

int is_mkv(char *bytes, int len)
{
    if(len < 4)
        return 0;
    return bytes[0] == 0x1a && bytes[1] == 0x45 && bytes[2] == 0xdf &&
        bytes[3] == 0xa3;
}

int is_avi(char *bytes, int len)
{
    if(len < 12)
        return 0;
    return bytes[0] == 0x52 && bytes[1] == 0x49 && bytes[2] == 0x46 && 
        bytes[3] == 0x46 && bytes[8] == 0x41 && bytes[9] == 0x56 &&
        bytes[10] == 0x49 && bytes[11] == 0x20;
}

int is_gif(char *bytes, int len)
{
    if(len < 6)
        return 0;
    return bytes[0] == 0x47 && bytes[1] == 0x49 && bytes[2] == 0x46 &&
        bytes[3] == 0x38 && (bytes[4] == 0x37 || bytes[4] == 0x39) && 
        bytes[5] == 0x61;
}

int is_mpg(char *bytes, int len)
{
    if(len < 4)
        return 0;
    return bytes[0] == 0x0 && bytes[1] == 0x0 && bytes[2] == 0x1 &&
        (bytes[3] == 0xba || bytes[3] == 0xb3);
}

int is_flac(char *bytes, int len)
{
    if(len < 4)
        return 0;
    return bytes[0] == 0x66 && bytes[1] == 0x4c && bytes[2] == 0x61 &&
        bytes[3] == 0x43;
}

int is_wav(char *bytes, int len)
{
    if(len < 12)
        return 0;
    return bytes[0] == 0x52 && bytes[1] == 0x49 && bytes[2] == 0x46 &&
        bytes[3] == 0x46 && bytes[8] == 0x57 && bytes[9] == 0x41 &&
        bytes[10] == 0x56 && bytes[11] == 0x45;
}

int is_mp3(char *bytes, int len)
{
    if(len < 3)
        return 0;
    return bytes[0] == 0x49 && bytes[1] == 0x44 && bytes[2] == 0x33;
}

int is_pdf(char *bytes, int len)
{
    if(len < 4)
        return 0;
    return bytes[0] == 0x25 && bytes[1] == 0x50 && bytes[2] == 0x44 &&
        bytes[3] == 0x46;
}

int is_jpg(char *bytes, int len)
{
    if(len < 12)
        return 0;
    return (bytes[0] == -1 && bytes[1] == -40 && bytes[2] == -1) &&
        ((bytes[3] == -37) ||
        (bytes[3] == -32 && bytes[4] == 0x0 && 
        bytes[5] == 0x10 && bytes[6] == 0x4a && bytes[7] == 0x46 && 
        bytes[8] == 0x49 && bytes[9] == 0x46 && bytes[10] == 0x0 &&
        bytes[11] == 0x1) || 
        (bytes[3] == -31 && bytes[6] == 0x45 && bytes[7] == 0x78 &&
        bytes[8] == 0x69 && bytes[9] == 0x66 && bytes[10] == 0x0 &&
        bytes[11] == 0x0));
}

int is_png(char *bytes, int len)
{
    if(len < 8)
        return 0;
    return bytes[0] == -119 && bytes[1] == 0x50 && bytes[2] == 0x4e &&
        bytes[3] == 0x47 && bytes[4] == 0xd && bytes[5] == 0xa &&
        bytes[6] == 0x1a && bytes[7] == 0xa;
}

int is_bmp(char *bytes, int len)
{
    if(len < 2)
        return 0;
    return bytes[0] == 0x42 && bytes[1] == 0x4d;
}

int is_gz(char *bytes, int len)
{
    if(len < 2)
        return 0;
    return bytes[0] == 0x1f && bytes[1] == -117;
}

int is_zip(char *bytes, int len)
{
    if(len < 4)
        return 0;
    return bytes[0] == 0x50 && bytes[1] == 0x4b &&
        ((bytes[2] == 0x3 && bytes[3] == 0x4) ||
        (bytes[2] == 0x5 && bytes[3] == 0x6) ||
        (bytes[2] == 0x7 && bytes[3] == 0x8)); 
}

int is_rar(char *bytes, int len)
{
    if(len < 8)
        return 0;
    return bytes[0] == 0x52 && bytes[1] == 0x61 && bytes[2] == 0x72 &&
        bytes[3] == 0x21 && bytes[4] == 0x1a && bytes[5] == 0x7 &&
        ((bytes[6] == 0x0) || (bytes[6] == 0x1 && bytes[7] == 0x0));
}

int is_bz2(char *bytes, int len)
{
    if(len < 3)
        return 0;
    return bytes[0] == 0x42 && bytes[1] == 0x5a && bytes[2] == 0x68;
}

int is_lz(char *bytes, int len)
{
    if(len < 4)
        return 0;
    return bytes[0] == 0x4c && bytes[1] == 0x5a && bytes[2] == 0x49 &&
        bytes[3] == 0x50;
}

int is_zlib(char *bytes, int len)
{
    if(len < 2)
        return 0;
    return bytes[0] == 0x78 && (bytes[1] == 0x1 || bytes[1] == -100 ||
            bytes[1] == -38);
}
