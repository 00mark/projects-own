#include <stdio.h>
#define MAXSIZE1 10000
#define MAXSIZE2 100
#define YES 1
#define NO 0

char s1[MAXSIZE1], s2[MAXSIZE2];

void gtString(char s[]){
    int i, c;
    for(i = 0; i < MAXSIZE1-1 && (c = getchar()) != EOF; ++i)
        s[i] = c;
    s[i] = '\0';
}

void squeeze(char s1[], char s2[]){
    int i, j, n, found;
    for(i = j = 0; s1[i] != '\0'; i++){
        found = NO;
        for(n = 0; s2[n] != '\0'; n++){
            if(s1[i] == s2[n])
                found = YES;
        }
        if(!found)
            s1[j++] = s1[i];
    }
    s1[j] = '\0';
}

main(){
    printf("line :\n> ");
    gtString(s1);
    printf("\nchars to delete :\n> ");
    gtString(s2);
    squeeze(s1, s2);
    printf("\nresult : %s", s1);
}
