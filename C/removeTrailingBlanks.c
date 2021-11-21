#include <stdio.h>
#define MAXLINE 1000
#define FOUND_LETTER 1
#define FOUND_WHITESPACE 0

main(){
    char line[MAXLINE];
    int c;
    int char_encountered = FOUND_WHITESPACE;
    while((c = getchar()) != EOF){
        if(c == '\n'){
            if(char_encountered == FOUND_LETTER)
                putchar(c);
            char_encountered = FOUND_WHITESPACE;
        }
        else if(char_encountered == FOUND_WHITESPACE && (c == ' ' || c == '\t'))
                ;
        else{
            char_encountered = FOUND_LETTER;
            putchar(c);
        }
    }
}
