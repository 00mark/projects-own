#include <stdio.h>
main(){
    int c, num_blanks, num_tabs, num_nls;
    num_blanks = num_tabs = num_nls = 0;
    while((c = getchar()) != EOF){
        if(c == ' ')
            num_blanks++;
        if(c == '\t')
            num_tabs++;
        if(c == '\n')
            num_nls++;
    }
    printf("blanks : %d, tabs : %d, newlines : %d", num_blanks, num_tabs,
            num_nls);
}
