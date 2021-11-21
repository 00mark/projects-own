#include <stdio.h>
#define BLANKS_FOR_TAB 8;

void replace(int num_blanks){
    for(int i = 0; i < num_blanks; i++){
        putchar(' ');
    }
}

main(){
    int c;
    int num_blanks = BLANKS_FOR_TAB;
    while((c = getchar()) != EOF){
        if(c == '\n'){
            putchar(c);
            num_blanks = BLANKS_FOR_TAB;
        }else if(c != '\t'){
            putchar(c);
            num_blanks--;
            if(num_blanks == 0)
                num_blanks = BLANKS_FOR_TAB;
        }else{
            replace(num_blanks);
            num_blanks = BLANKS_FOR_TAB;
        } 
    }
}
