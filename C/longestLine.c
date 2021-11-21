#include <stdio.h>
#define MAXLINE 1000

int getline2(char s[], int lim){
    int c, i;
    for(i = 0; i < lim-1 && (c = getchar()) != EOF && c != '\n'; i++)
        s[i] = c;
    if(c == '\n'){
        s[i] = c;
        ++i;
    }
    s[i] = '\0';
    return i;
}

void copy(char to[], char from[]){
    int i = 0;
    while((to[i] = from[i]) != '\0')
        ++i;
}

main(){
    int len, max, current_max;
    current_max = MAXLINE;
    char line[current_max], longest[current_max];
    max = 0;
    while((len = getline2(line, current_max)) > 0)
        if(len > max){
            max = len;
            if(len > current_max){
                current_max *= 2;
                char longest[current_max];
                copy(longest, line);
                //longest = tmp;
            }else
                copy(longest, line);
        }
    if(max > 0)
        printf("[%d] %s", max, longest);
    return 0;
}
