#include <stdio.h>
#define SUCCESS 1
#define NO_SUCCESS 0
#define QUOTE 1
#define NO_QUOTE 0
#define COMMENT 1
#define NO_COMMENT 0
#define CRITICAL_C_LENGTH 7
#define ESCAPED 1
#define NOT_ESCAPED 0
#define MISTAKE_OPEN 1
#define MISTAKE_CLOSE 0

int mistake;
int lines = 1;
int status = SUCCESS;
int quote_status = NO_QUOTE;
int comment_status = NO_COMMENT;
int escaped = NOT_ESCAPED;
char critical_characters[7] = {'(', '[', '{', '/', '*', '\'', '"'};
char resolving_characters[7] = {')', ']', '}', '\n', '*', '\'', '"'};

int check(int encountered_c){
    int c;
    int tmp_lines = 0;
    char last_c = '\0';
    //printf("\n%c, comment: %d, quote: %d", critical_characters[encountered_c],
    //        comment_status, quote_status);
    while((c = getchar()) != EOF && status == SUCCESS){
        //printf("%c ", c);
        if(c == '\n')
            tmp_lines++;
        if(c == '\\'){
            c = getchar();
            last_c = '\0';
        }else if(last_c == '*'){
            if(c == '/'){
                comment_status = NO_COMMENT;
                lines += tmp_lines;
                return SUCCESS;
            }
            last_c = c;
        }else if(c == resolving_characters[encountered_c]){
            if(c == '*'){
                last_c = c;
                continue;
            }
            if(c == '\n')
                comment_status = NO_COMMENT;
            if(c == '\'' || c == '"')
                quote_status = NO_QUOTE;
            lines += tmp_lines;
            return SUCCESS;
        }else if(c == '*' && last_c != '/')
            ;
        else if(quote_status == QUOTE || comment_status == COMMENT)
            ;
        else{
            if(last_c == '/' && c != '*' && c != '/')
                last_c = c;
            for(int i = 0; i < CRITICAL_C_LENGTH; i++){
                if(i < 3 && c == resolving_characters[i]){
                    mistake = MISTAKE_CLOSE;
                    return -i;
                }
                if(c == critical_characters[i]){
                    if(c == '/' && last_c != '/'){
                        last_c = '/';
                        break;
                    }
                    if(c == '*' || c == '/')
                        comment_status = COMMENT;
                    if(c == '\'' || c == '"')
                        quote_status = QUOTE;
                    status = check(i);
                    last_c = '\0';
                    break;
                }
            }
        }
    }
    if(critical_characters[encountered_c] == '/'){
        lines += tmp_lines;
        return SUCCESS;
    }
    mistake = MISTAKE_OPEN;
    return -encountered_c;
}
main(){
    int c, last_c;
    last_c = '\0';
    while((c = getchar()) != EOF && status == SUCCESS){
        if(c == '\n')
            lines++;
        if(c == '*' && last_c != '/')
            ;
        else if(c == '\\')
            c = getchar();
        else{
            for(int i = 0; i < CRITICAL_C_LENGTH; i++){
                if(c == critical_characters[i]){
                    if(c == '/' && last_c != '/'){
                        last_c = '/'; 
                        break;
                    }
                    quote_status = c == '\'' || c  == '"' ? QUOTE : NO_QUOTE;
                    comment_status = last_c == '/' && (c == '/' || c == '*')  ? 
                        COMMENT : NO_COMMENT;
                    last_c = '\0';
                    status = check(i);
                    break;
                }else if(i < 3 && c == resolving_characters[i]){
                    mistake = MISTAKE_CLOSE;
                    status = -i;
                }
            }
        }
    }
    if(status == SUCCESS)
        printf("\nSUCCESS");
    else{
        printf("\nNO SUCCESS : line %d : %c expected", lines, 
                (mistake == MISTAKE_OPEN ? resolving_characters[-status] :
                 critical_characters[-status]));
    }
}
