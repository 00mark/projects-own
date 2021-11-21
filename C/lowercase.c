#include <stdio.h>

int lower(int c){
    return c < 91 && c > 64 ? c - ('A' - 'a') : c;
}

void printlower(){
    int c;
    while((c = getchar()) != EOF)
        putchar(lower(c));
}

main(){
    printlower();
}
