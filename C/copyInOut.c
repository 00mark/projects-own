#include <stdio.h>

main(){
    int c;
    int blank_found = 0;
    while((c = getchar()) != EOF){
        if(c == ' '){
            if(blank_found == 0){
                blank_found = 1;
                putchar(c);
            }
            else
                continue;
        }else{
            putchar(c);
            blank_found = 0;
        }
    }
}
