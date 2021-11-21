#include <stdio.h>
#define MAXSIZE 10000
#define FOUND 1
#define NOT_FOUND 0

int isnumlet(char c){
    if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')
            || (c >= 'a' && c <= 'z'))
        return 1;
    return 0;
}

int putchars(char s[], int j, char first, char last){
    if(first == last)
        return j;
    first < last ? first++ : first--;
    for(; first != last; (first < last ? first++ : first--), j++){
        if(!isnumlet(first)){
            if(first < last)
                first = 'a';
            else
                first = 'Z';
        }
        s[j] = first;
    }
    s[j] = first;
    return j;
}

void expand(char s1[], char s2[]){
    int i, j, minus;
    char c = '\0';
    minus = NOT_FOUND;
    for(i = j = 0; s1[i] != '\0' && j < MAXSIZE-1; i++, j++){
        if(minus){
            if(isnumlet(s1[i])){
                if(s1[i] <= '9'){
                    if(c <= '9')
                        j = putchars(s2, j, c, s1[i]);
                    else{
                        s2[j] = '-';             
                        j++;
                        s2[j] = s1[i];
                    }
                }else{
                    if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
                        j = putchars(s2, j, c, s1[i]);
                    else{
                        s2[j] = '-';
                        j++;
                        s2[j] = s1[i];
                    }
                }
            } 
            minus = NOT_FOUND;
            c = s1[i];
        }else if(isnumlet(c) && s1[i] == '-'){
            minus = FOUND; 
            j--;
        }else{
            s2[j] = s1[i];
            c = s1[i];
        }
    }
    s2[j] = '\0';
}

main(){
    char s1[] = "1-7-4a-yZ-B"; 
    char s2[MAXSIZE];
    expand(s1, s2);
    printf("%s\n", s1);
    printf("%s", s2);
}
