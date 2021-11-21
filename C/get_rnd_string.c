#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_LENGTH 1000
#define MAX_LENGTH_STRING "1000"

void die(const char *msg)
{
    printf("%s\n", msg);
    printf("usage: get_rnd_string length\n");
    exit(-1);
}

int char_is_valid(char c)
{
    return (c == '!') || (c >= '#' && c <= '&') || (c >= '(' && c <= '+') ||
        (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z');
}

void get_string(char *string, int len)
{
    FILE *fp;
    int i;
    char c;

    if(!(fp = fopen("/dev/urandom", "r")))
        die("could not open /dev/urandom");

    for(i = 0; i < len; i++){
        fread((void *)&c, 1, 1, fp); 
        if(char_is_valid(c))
            string[i] = c;
        else
            i--;
    }
    string[len] = '\0';

    fclose(fp);
}

int main(int argc, char **argv)
{
    int len; 
    if(argc != 2)
        die("no length specified");
    if(!(len = atoi(argv[1])))
        die("invalid length argument");
    if(len > MAX_LENGTH)
        die("string length should not surpass " MAX_LENGTH_STRING);
    char string[argc + 1];

    get_string(string, len);
    printf("%s\n", string);

    return 0;
}
