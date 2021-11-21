#include <stdio.h>

#define IN  1
#define OUT 0

main(){
    int in_word, c;
    while((c = getchar()) != EOF){
        if(c == ' ' || c == '\t' || c == '\n'){
            if(in_word == OUT)
                continue;
            else
                in_word = OUT;
            putchar('\n');
        }
        else{
            in_word = IN;
            putchar(c);
        }
    }
}
