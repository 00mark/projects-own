#include <stdio.h>
#define MAXSIZE 1000

void reverse(char s[], int length){
    for(int i = 0, j = length-1; i < j; i++, j--){
        char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }
}

int gtline(char s[], int lim){
    int c, i;
    for(i = 0; i < lim-1 && (c = getchar()) != EOF; i++){
        s[i] = c;
        if(c == '\n'){
            break;
        }
    } 
    s[i+1] = '\0';
    return i;
}

void printline(char l[]){
    for(int i = 0; l[i] != '\0'; i++)
        putchar(l[i]);
}

main(){
    int length;
    char line[MAXSIZE];
    while((length = gtline(line, MAXSIZE)) != 0){
        reverse(line, length);
        printline(line);
    }
}
