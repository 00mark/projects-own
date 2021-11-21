#include <stdio.h>
#define MAXSIZE1 10000
#define MAXSIZE2 100

char s1[MAXSIZE1], s2[MAXSIZE2];

void gtString(char s[]){
    int i, c;
    for(i = 0; i < MAXSIZE1-1 && (c = getchar()) != '\n'; i++)
        s[i] = c;
    s[i] = '\0';
}

int any(char s1[], char s2[]){
    int location = 1;
    for(int i = 0; s1[i] != '\0'; i++){
        for(int j = 0; s2[j] != '\0'; j++)    
            if(s1[i] == s2[j])
                return i+1;
    }
    return -1;
}

main(){
    printf("String1 :\n> ");
    gtString(s1);
    printf("\nString2 :\n> ");
    gtString(s2);
    printf("\nPos : %d", any(s1, s2));
}
