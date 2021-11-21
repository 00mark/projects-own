#include <stdio.h>
#include <string.h>
#define MAXSIZE 30

void reverse(char s[]){
    for(int i = 0, j = strlen(s)-1; i < j; i++, j--){
        char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }
}

void itoa(int n, char s[]){
    int i, sign;
    sign = 1;
    i = 0;
    if(n < 0)
        sign = -1;
    do{
        s[i++] = -(n % 10 - 10) % 10+ '0';
    }while((n /= 10) != 0);
    if(sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

main(){
    int n = -1000;
    char s[MAXSIZE];
    itoa(n, s);
    printf("before : %d, after : %s", n, s);
}
