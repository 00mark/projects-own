#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSIZE 1024

void reverse(unsigned char s[], int i1, int i2){
    unsigned char tmp;
    
    for(; i1 < i2; i1++, i2--){
        tmp = s[i1];
        s[i1] = s[i2];
        s[i2] = tmp;
    }
}

void itob(int n, unsigned char s[], int b){
    int negativ, i;

    if(b <= 0){
        strcpy(s, "invalid");
        return;
    }
    negativ = n >= 0 ? 0 : 1;
    i = 0;
    if(negativ){
        n = -n;
        s[i++] = '-';
    }
    if(b == 1){
        for(; i < MAXSIZE - 1 && i < n; i++)
            s[i] = '|';
        s[i] = '\0';
        return;
    }
    while(n > 0){
        s[i++] = (n % b > 9 ? (n % b) - 10 + 'A' : n % b + '0');
        n /= b;
    }
    reverse(s, negativ, i - 1);
    s[i] = '\0';
}

int main(int argc, unsigned char **argv){
    unsigned char s[MAXSIZE];
    int n, b;

    if(argc == 3){
        n = atoi(argv[1]);
        b = atoi(argv[2]);
    }else{
        n = 157143;
        b = 16;
    }
    itob(n, s, b);
    printf("decimal: %d, base%d: %s\n", n, b, s);
}
