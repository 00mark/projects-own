#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ERR_INVAL -1
#define ERR_ARGS -2
#define ERR_FILE -3
#define WRAP_COUNT 81
#define FILE_LENGTH 2048
#define BASE64 1
#define BASE32 2
#define BASE16 3

const char enc16_map[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                            'A', 'B', 'C', 'D', 'E', 'F'};

const char dec16_map[128] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                             -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                             -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                             -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4,
                             5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1, -1, 10, 11,
                             12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                             -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                             -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1,
                             -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                             -1, -1, -1, -1, -1, -1, -1, -1, -1};


const char enc32_map[32] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 
                            'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                            'U', 'V', 'W', 'X', 'Y', 'Z', '2', '3', '4', '5',
                            '6', '7'};

const int dec32_map[128] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 26, 27,
                            28, 29, 30, 31, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
                            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1};


const char enc64_map[64] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 
                            'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                            'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
                            'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                            'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
                            'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', '+', '/'};

const int dec64_map[128] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55,
                            56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1,
                            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
                            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1,
                            -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34,
                            35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
                            48, 49, 50, 51, -1, -1, -1, -1, -1};

int size;
int err = 0;

void usage(){
    printf("Usage: baseX [-h] [-16|-32|-64] [-d [-i]|-w] File\n");
}

void help(){
    usage();
    printf("\t-h  [--help] show this help message\n\t-16 [--base16] use base16 "
           "encoding\n\t-32 [--base32] use base32 encoding\n\t-64 [--base64] "
           "use base64 encoding (default)\n\t-d  [--decode] decode file\n\t-i  "
           "[--ignore_newline] ignore newlines while decoding\n\t-w  [--wrap] "
           "wrap lines while encoding\n");
}

unsigned char *dec16(unsigned char *input){
    unsigned char *ret;
    int ret_size, i, j;

    ret_size = size * 0.5 + 1;
    ret = malloc(ret_size);
    i = j = 0;
    while(i < size){
        if(j >= ret_size + 1){
            ret_size *= 2;
            ret = realloc(ret, ret_size);
        }
        if(input[i] == '\0' || input[i] == '=')
            break;
        if(input[i] > 127 || dec16_map[input[i]] == -1){
            err = ERR_INVAL;
        }
        if(input[i + 1] == '\0' || input[i] == '='){
            ret[j++] = dec16_map[input[i]] << 4;
            ret[j++] = '=';
            break;
        }else{
            if(input[i + 1] > 127 || dec16_map[input[i + 1]] == -1){
                err = ERR_INVAL;
                return ret;
            }
            ret[j++] = (dec16_map[input[i]] << 4) + dec16_map[input[++i]];
            i++;
        }
    }
    if(j >= ret_size)
        ret = realloc(ret, ret_size + 1);
    ret[j] = '\0';
    size = j;

    return ret;
}

unsigned char *enc16(unsigned char *input){
    unsigned char *ret;
    int ret_size, i, j;

    ret_size = size * 2 + 1;
    ret = malloc(ret_size);
    i = j = 0;
    while(i < size){
        if(j >= ret_size + 2){
            ret_size *= 2;
            ret = realloc(ret, ret_size);
        }
        ret[j++] = enc16_map[input[i] >> 4];
        ret[j++] = enc16_map[input[i++] % 16];
    }
    if(j >= ret_size)
        ret = realloc(ret, ret_size + 1);
    ret[j] = '\0';
    size = j;

    return ret;
}

unsigned char *dec32(unsigned char *input){
    unsigned char *ret;
    int ret_size, i, j;

    ret_size = size * 0.8;
    ret = malloc(ret_size);
    i = j = 0;
    while(i < size){
        if(j + 7 >= ret_size ){
            ret_size *= 2;
            ret = realloc(ret, ret_size);
        }
        if(input[i] == '\0' || input[i] == '=')
            break;
        if(input[i] > 127 || dec32_map[input[i]] == -1){
            err = ERR_INVAL;
            return ret;
        }
        if(input[i + 1] == '\0' || input[i + 1] == '='){
            ret[j++] = dec32_map[input[i]] << 3;
            break;
        }else{
            if(input[i + 1] > 127 || dec32_map[input[i + 1]] == -1){
                err = ERR_INVAL;
                return ret;
            }
            ret[j++] = (dec32_map[input[i]] << 3) + 
                (dec32_map[input[++i]] >> 2);
        }
        if(input[i + 1] == '\0' || input[i + 1] == '='){
            ret[j++] = (dec32_map[input[i]] << 6) % 256;
            break;
        }else if(input[i + 2] == '\0' || input[i + 2] == '='){
            if(input[i + 1] > 127 || dec32_map[input[i + 1]] == -1){
                err = ERR_INVAL;
                return ret;
            }
            ret[j++] = ((dec32_map[input[i]] << 6) % 256) +
                (dec32_map[input[++i]] << 1);
            break;
        }else{
            if(input[i + 1] > 127 || dec32_map[input[i + 1]] == -1 ||
                    input[i + 2] > 127 || dec32_map[input[i + 2]] == -1){
                err = ERR_INVAL;
                return ret;
            }
            ret[j++] = ((dec32_map[input[i]] << 6) % 256) +
                (dec32_map[input[++i]] << 1) + (dec32_map[input[++i]] >> 4);
        }
        if(input[i + 1] == '\0' || input[i + 1] == '='){
            ret[j++] = (dec32_map[input[i]] << 4) % 256;
            break;
        }else{
            if(input[i + 1] > 127 || dec32_map[input[i + 1]] == -1){
                err = ERR_INVAL;
                return ret;
            }
            ret[j++] = ((dec32_map[input[i]] << 4) % 256) +
                (dec32_map[input[++i]] >> 1);
        }
        if(input[i + 1] == '\0' || input[i + 1] == '='){
            ret[j++] = (dec32_map[input[i]] << 7) % 256;
            break;
        }else if(input[i + 2] == '\0' || input[i + 2] == '='){
            if(input[i + 1] > 127 || dec32_map[input[i + 1]] == -1){
                err = ERR_INVAL;
                return ret;
            }
            ret[j++] = ((dec32_map[input[i]] << 7) % 256) +
                (dec32_map[input[++i]] << 2);
            break;
        }else{
            if(input[i + 1] > 127 || dec32_map[input[i + 1]] == -1 ||
                    input[i + 2] > 127 || dec32_map[input[i + 2]] == -1){
                err = ERR_INVAL;
                return ret;
            }
            ret[j++] = ((dec32_map[input[i]] << 7) % 256) + 
                (dec32_map[input[++i]] << 2) + (dec32_map[input[++i]] >> 3);
        }
        if(input[i + 1] == '\0' || input[i + 1] == '='){
            ret[j++] = (dec32_map[input[i]] << 5) % 256;
            break;
        }else{
            if(input[i + 1] > 127 || dec32_map[input[i + 1]] == -1){
                err = ERR_INVAL;
                return ret;
            }
            ret[j++] = ((dec32_map[input[i]] << 5) % 256) +
                dec32_map[input[++i]];
        }
        i++;
    }
    if(j == ret_size)
        ret = realloc(ret, ret_size + 1);
    ret[j] = '\0';
    size = j;

    return ret;
}

unsigned char *enc32(unsigned char *input){
    unsigned char *ret;
    int ret_size, i, j;

    ret_size = size * 2;
    ret = malloc(ret_size);
    i = j = 0;
    while(i < size){
        if(j + 7 >= ret_size){
            ret_size *= 2;
            ret = realloc(ret, ret_size);
        }
        ret[j++] = enc32_map[input[i] >> 3];
        if(i + 1 >= size){
            ret[j++] = enc32_map[(input[i] << 2) % 32]; 
            memset(ret + j, '=', 6);
            j += 6;
            break;
        }else
            ret[j++] = enc32_map[((input[i] << 2) % 32) + (input[++i]
                    >> 6)];
        ret[j++] = enc32_map[(input[i] >> 1) % 32];
        if(i + 1 >= size){
            ret[j++] = enc32_map[(input[i] << 4) % 32];
            memset(ret + j, '=', 4);
            j += 4;
            break;
        }else
            ret[j++] = enc32_map[((input[i] << 4) % 32) + (input[++i]
                    >> 4)];
        if(i + 1 >= size){
            ret[j++] = enc32_map[(input[i] << 1) % 32];
            memset(ret + j, '=', 3);
            j += 3;
            break;
        }else
            ret[j++] = enc32_map[((input[i] << 1) % 32) + (input[++i]
                    >> 7)];
        ret[j++] = enc32_map[(input[i] >> 2) % 32];
        if(i + 1 >= size){
            ret[j++] = enc32_map[(input[i] << 3) % 32];
            ret[j++] = '=';
            break;
        }else
            ret[j++] = enc32_map[((input[i] << 3) % 32) + (input[++i] >> 5)];
        ret[j++] = enc32_map[input[i++] % 32];
    }
    if(j == ret_size)
        ret = realloc(ret, ret_size + 1);
    ret[j] = '\0';
    size = j;

    return ret;
}

unsigned char *dec64(unsigned char *input){
    unsigned char *ret;
    int ret_size, i, j;

    ret_size = size * 0.8;
    ret = malloc(ret_size);
    i = j = 0;
    while(i < size){
        if(j + 3 >= ret_size){
            ret_size *= 2;
            ret = realloc(ret, ret_size);
        }
        if(input[i] == '\0' || input[i] == '=')
            break;
        if(input[i] > 127 || dec64_map[input[i]] == -1){
            err = ERR_INVAL;
            return ret;
        }
        if(input[i + 1] == '\0' || input[i + 1] == '='){
            ret[j++] = dec64_map[input[i]] << 2; 
            break;
        }else{
            if(input[i + 1] > 127 || dec64_map[input[i + 1]] == -1){
                err = ERR_INVAL;
                return ret;
            }
            ret[j++] = (dec64_map[input[i]] << 2) + (dec64_map[input[++i]]
                    >> 4);
        }
        if(input[i + 1] == '\0' || input[i + 1] == '='){
            ret[j++] = (dec64_map[input[i]] % 16) << 4; 
            break;
        }else{
            if(input[i + 1] > 127 || dec64_map[input[i + 1]] == -1){
                err = ERR_INVAL;
                return ret;
            }
            ret[j++] = ((dec64_map[input[i]] % 16) << 4) + (dec64_map[input[++i 
                    ]] >> 2);
        }
        if(input[i + 1] == '\0' || input[i + 1] == '='){
            ret[j++] = (dec64_map[input[i]] % 4) << 6;
            break;
        }else{
            if(input[i + 1] > 127 || dec64_map[input[i + 1]] == -1){
                err = ERR_INVAL;
                return ret;
            }
            ret[j++] = ((dec64_map[input[i]] % 4) << 6) + dec64_map[input[++i]];
        }
        i++;
    }
    if(j == size)
        ret = realloc(ret, size + 1);
    ret[j] = '\0';
    size = j;

    return ret;
}

unsigned char *enc64(unsigned char *input){
    unsigned char *ret;
    int ret_size, i, j;

    ret_size = size * 1.5;
    ret = malloc(ret_size);
    i = j = 0;
    while(i < size){
        if(j + 3 >= ret_size){
            ret_size *= 2;
            ret = realloc(ret, ret_size);
        }
        ret[j++] = enc64_map[input[i] >> 2]; 
        if(i + 1 >= size){
            ret[j++] = enc64_map[(input[i] << 4) % 64];
            memset(ret + j, '=', 2);
            j += 2;
            break;
        }else
            ret[j++] = enc64_map[((input[i] << 4) % 64) + (input[++i] 
                    >> 4)];
        if(i + 1 >= size){
            ret[j++] = enc64_map[(input[i] << 2) % 64];
            ret[j++] = '=';
            break;
        }else
            ret[j++] = enc64_map[((input[i] << 2) % 64) + (input[++i] 
                    >> 6)];
        ret[j++] = enc64_map[input[i++] % 64];
    }
    if(j == ret_size)
        ret = realloc(ret, ret_size + 1);
    ret[j] = '\0';
    size = j;

    return ret;
}

unsigned char *get_content(FILE *fp, int ignore_nl){
    unsigned char *ret;
    int i, sz, c;

    ret = malloc(FILE_LENGTH);
    for(i = 0, sz = FILE_LENGTH; (c = fgetc(fp)) != EOF; i++){
        if(i == sz){
            sz *= 2;
            ret = realloc(ret, sz);
        }
        if(c == '\n' && ignore_nl)
            i--;
        else
            ret[i] = c;
    }
    if(i == sz)
        ret = realloc(ret, sz + 1);
    ret[i] = '\0';
    size = i;

    return ret;
}

void print_result(unsigned char *result, int wrap){
    int i, x;

    for(i = 0, x = 1; i < size - 1; i++){
        if(wrap && !((i + x) % WRAP_COUNT)){
            x++;
            putchar('\n');
        }
        putchar(result[i]);
    }
    putchar('\n');
}

int main(int argc, char **argv){
    FILE *fp;
    unsigned char *content, *result;
    int decode, i, base, ignore_nl, wrap, file_index;

    decode = base = ignore_nl = wrap = file_index = 0;
    for(i = 1; i < argc; i++){
        if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")){
            help();
            return 0;
        }else if(!decode && (!strcmp(argv[i], "-d") || !strcmp(argv[i], 
                    "--decode")))
            decode = 1;
        else if(!base && (!strcmp(argv[i], "-16") || !strcmp(argv[i],
                    "--base16")))
            base = BASE16;
        else if(!base && (!strcmp(argv[i], "-32") || !strcmp(argv[i],
                    "--base32")))
            base = BASE32;
        else if(!base && (!strcmp(argv[i], "-64") || !strcmp(argv[i],
                    "--base64")))
            base = BASE64;
        else if(!ignore_nl && (!strcmp(argv[i], "-i") || !strcmp(argv[i],
                    "--ignore_newline")))
            ignore_nl = 1;
        else if(!wrap && (!strcmp(argv[i], "-w") || !strcmp(argv[i], "--wrap")))
            wrap = 1;
        else if(!file_index)
            file_index = i;
        else{
            usage();
            return ERR_ARGS;
        }
    }
    if(!base)
        base = BASE64;
    if((!decode && ignore_nl) || (decode && wrap)){
        usage();
        return ERR_ARGS;
    }
    if(file_index == 0 || !strcmp(argv[file_index], "-"))
        content = get_content(stdin, ignore_nl);
    else if((fp = fopen(argv[file_index], "r")) == NULL){
        printf("File not found\n");
        return ERR_FILE;
    }else{
        content = get_content(fp, ignore_nl);
        fclose(fp);
    }
    if(decode)
        result = base == BASE64 ? dec64(content) : base == BASE32 ? dec32(
                content) : dec16(content);
    else
        result = base == BASE64 ? enc64(content) : base == BASE32 ? enc32(
                content) : enc16(content);
    if(err != 0)
        printf("Error while decoding: Invalid Input\n");
    else
        print_result(result, wrap);
    free(content);
    free(result);    

    return 0;
}
