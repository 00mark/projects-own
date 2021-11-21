#include <stdio.h>
#define MIN 80
#define MAX 1000

main(){
    int length = MAX;
    char line[length];
    int c, current_length;
    for(int current_length = 0; (c = getchar()) != EOF; current_length++){
        if(c != '\n'){
            line[current_length] = c;
            if(current_length > length)
                break;
        }else{
            if(current_length > MIN){
                printf("[%d] ", current_length);
                for(int i = 0; i < current_length; i++)
                    putchar(line[i]);
                printf("\n");
            }
            current_length = -1;
            char line[length];
        }
    }
}
