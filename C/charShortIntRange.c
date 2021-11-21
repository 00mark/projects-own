#include <stdio.h>
#define VALUE_SET 1

main(){
    int i, prev_int, max_int, min_int; 
    short s, prev_short, max_short, min_short, short_status;
    char c, prev_char, max_char, min_char, char_status;
    char_status = short_status = 0;
    
    i = s = c = 1;
    
    for(; i > 0; i++, s++, c++){
        if(c <= 0 && char_status != VALUE_SET){
            char_status = VALUE_SET;
            max_char = prev_char;
            min_char = c;
        }
        if(s <= 0 && short_status != VALUE_SET){
            short_status = VALUE_SET;
            max_short = prev_short;
            min_short = s;
        }
        prev_int = i;
        prev_short = s;
        prev_char = c;
    }
    max_int = prev_int;
    min_int = i;
    
    printf("int : min %d, max %d\n", min_int, max_int);
    printf("short: min %d, max %d\n", min_short, max_short);
    printf("char: min %d, max %d\n", min_char, max_char);
}
