#include <stdio.h>
#define FOLD_COLUMN 80
#define BLANKS_FOR_TAB 4
#define MAX_CHARS 1000

int foldLine(){
    int c, length, num_tabs_blanks, num_folds, word_index;
    char tabs_blanks[MAX_CHARS], word[FOLD_COLUMN];
    length = num_tabs_blanks = num_folds = word_index = 0;
    while((c = getchar()) != '\n' && c != EOF){
        if(c == ' '){
            if(word_index > 0){
                if(length >= FOLD_COLUMN){
                    printf("\n");
                    length = 0;
                    num_tabs_blanks = 0;
                    num_folds++;
                    for(int i = 0; i < word_index; i++){
                        putchar(word[i]);
                        length++;
                    }
                }else
                    for(int i = 0; i < word_index; i++)
                        putchar(word[i]);
            }
            length++;
            tabs_blanks[num_tabs_blanks] = ' ';
            num_tabs_blanks++;
            word_index = 0;
        }
        else if(c == '\t'){
            if(word_index > 0){
                if(length >= FOLD_COLUMN){
                    printf("\n");
                    length = 0;
                    num_tabs_blanks = 0;
                    num_folds++;
                    for(int i = 0; i < word_index; i++){
                        putchar(word[i]);
                        length++;
                    }
                }else
                    for(int i = 0; i < word_index; i++)
                        putchar(word[i]);
            }
            length += BLANKS_FOR_TAB - (length % BLANKS_FOR_TAB);
            tabs_blanks[num_tabs_blanks] = '\t';
            num_tabs_blanks++;
            word_index = 0;
        }
        else{
            if(word_index == FOLD_COLUMN-1){
                for(int i = 0; i < FOLD_COLUMN-1; i++)
                    putchar(word[i]);
                printf("-\\\n");
                length = 0;
                word_index = 0;
                num_folds++;
            }else if(length >= FOLD_COLUMN){
                printf("\n");
                length = 0;
                for(int i = 0; i < word_index; i++){
                        putchar(word[i]);
                        length++;
                }
                word_index = 0;
                num_folds++;
            }else{
                for(int i = 0; i < num_tabs_blanks; i++)
                    putchar(tabs_blanks[i]);
            }
            num_tabs_blanks = 0;
            word[word_index] = c;
            word_index++;
            length++;
        }
    }
    for(int i = 0; i < word_index; i++)
        putchar(word[i]);
    if(c == '\n')
        length++;
    return length;
}

main(){
    int len;
    while((len = foldLine()) != 0)
        putchar('\n');
}
