#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argv, char **argc){
    int c, extended, occurances[40];

    extended = 0;
    if(argv >= 2)
        extended = atoi(argc[1]);
    for(int i = 0; i < 40; i++)
        occurances[i] = 0;
    while((c = getchar()) != EOF){
        if(c >= '0' && c <= '9')
            occurances[c-'0']++;
        else if(c >= 'a' && c <= 'z')
            occurances[c-('a'-10)]++;
        else if(c >= 'A' && c <= 'Z')
            occurances[c-('A'-10)]++;
        else if(c == ' ')
            occurances[36]++;
        else if(c == '\t')
            occurances[37]++;
        else if(c == '\n')
            occurances[38]++;
        else
            occurances[39]++;
    }
    for(int i = 0; i < 40; i++){
        if(i < 36)
            i < 10 ? printf("%d\t", i) : printf("%c\t", 'a' + (i-10));
        else if(i > 38)
            printf("other\t");
        else 
            i == 36 ? printf(" \t") : i == 37 ? printf("\\t\t") :
                printf("\\n\t");
        if(extended == 1){
            for(int j = 0; j < occurances[i]; j++)
                printf("░");
        }
        occurances[i] == 0 ? : printf(" [%d]", occurances[i]);
        printf("\n");
    }
}
