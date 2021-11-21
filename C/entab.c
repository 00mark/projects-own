#include <stdio.h>
#define BLANKS_FOR_TAB 4

void replace(int num_blanks, int num_chars){
    int pos = num_chars % BLANKS_FOR_TAB; 
    while(num_blanks >= (BLANKS_FOR_TAB - pos)){
        putchar('\t');
        num_blanks -= (BLANKS_FOR_TAB - pos);
        pos = 0;
    }
    for(; num_blanks > 0; num_blanks--)
        putchar(' ');
}

main(){
    int c, consecutive_blanks, num_chars, encountered_blank;
    consecutive_blanks = num_chars = encountered_blank = 0;
    while((c = getchar()) != EOF){
        num_chars++;
        if(c == '\n'){
            consecutive_blanks == 0 ? : replace(consecutive_blanks, 
                    encountered_blank);
            consecutive_blanks = num_chars = 0;
            putchar(c);
        }else if(c == ' '){
            if(consecutive_blanks == 0)
                encountered_blank = num_chars-1;
            consecutive_blanks++;
        }else{ 
            consecutive_blanks == 0 ? : replace(consecutive_blanks, 
                    encountered_blank);
            putchar(c);
            consecutive_blanks = 0;
        }
    }
}
