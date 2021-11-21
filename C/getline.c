#include <stdio.h>
#define MAXLINE 1000

int lim = MAXLINE;
char s[MAXLINE];

void gtline(){
    int i, c;
    i = 0;
    while(i < lim-1){
        c = getchar();
        if(c == '\n')
            break;
        if(c == EOF)
            break;
        s[i] = c;
        i++;
    }
    s[i] = '\0';
}

void printline(){
    for(int i = 0; s[i] != '\0'; i++)
        putchar(s[i]);
}

main(){
    gtline();
    printline();
}


