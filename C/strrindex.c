#include <stdio.h>
#include <string.h>

#define MAXSTRING 50

int strrindex(char s[], char t[]){
    int i, j, k;

    for(i = strlen(s) - 1; i > strlen(t) - 1; i--){
            for(j = strlen(t) - 1, k = i; j >= 0 && s[k] == t[j]; j--, k--);
        if(j < 0)
            return k + 2;
    }
    return -1;
}

int main(){
    char s[] = "abcd as dfa sdffd fas dfd"; 
    char t[] = "s";

    printf("\"%s\" at position %d in \"%s\"\n", t, strrindex(s, t), s);
    return 0;
}
