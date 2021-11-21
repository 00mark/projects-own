#include <stdio.h>
#define COMMENT 2
#define LINE_COMMENT 1
#define COMMENT_POSSIBLE 0
#define COMMENT_END_POSSIBLE -1
#define NO_COMMENT -2
#define QUOTE 1
#define NO_QUOTE 0

void remove_comments(){
    int c, last_c, comment_status, single_quote_status, double_quote_status;
    single_quote_status = double_quote_status = NO_QUOTE;
    comment_status = NO_COMMENT;
    last_c = '\0';
    while((c = getchar()) != EOF){
        if(c == '/' && single_quote_status == NO_QUOTE && double_quote_status ==
                NO_QUOTE && comment_status == NO_COMMENT){
            last_c = c;
            comment_status = COMMENT_POSSIBLE;
        }else if(c == '*' && comment_status == COMMENT_POSSIBLE)
            comment_status = COMMENT;
        else if(c == '/' && comment_status == COMMENT_POSSIBLE)
            comment_status = LINE_COMMENT;
        else if(c == '/' && comment_status == COMMENT_END_POSSIBLE){
            comment_status = NO_COMMENT;
            last_c = '\0';
        }else if(comment_status == COMMENT_END_POSSIBLE)
            comment_status = COMMENT;
        else if(c == '*' && comment_status == COMMENT){
            comment_status = COMMENT_END_POSSIBLE;
            last_c = c;
        }else if(comment_status == COMMENT || (comment_status == LINE_COMMENT &&
                    c != '\n'));
        else{
            if(c == '\n' && comment_status == LINE_COMMENT)
                comment_status = NO_COMMENT;
            else if(c == '\''){
                if(single_quote_status == QUOTE)
                    single_quote_status = NO_QUOTE;
                else if(single_quote_status == NO_QUOTE && 
                        double_quote_status == NO_QUOTE && (comment_status == 
                        NO_COMMENT || comment_status == COMMENT_POSSIBLE))
                    single_quote_status = QUOTE;
            }else if(c == '"'){
                if(double_quote_status == QUOTE)
                    double_quote_status = NO_QUOTE;
                else if(single_quote_status == NO_QUOTE && 
                        double_quote_status == NO_QUOTE && (comment_status == 
                        NO_COMMENT || comment_status == COMMENT_POSSIBLE))
                    double_quote_status = QUOTE;
            }else if(comment_status == COMMENT_POSSIBLE){
                comment_status = NO_COMMENT;
                putchar(last_c);
            }
            putchar(c);
        }
    }
}

main(){
    remove_comments();
}
