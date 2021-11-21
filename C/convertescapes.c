#include <stdio.h>
#define MAXSIZE 1000

void escape(char s[], char t[]){
    int i, j;
    for(i = j = 0; j < MAXSIZE-2 && s[i] != '\0'; i++, j++){
        switch(s[i]){
            case '\n':
                t[j] = '\\';
                j++;
                t[j] = 'n'; 
                break;
            case '\t':
                t[j] = '\\';
                j++;
                t[j] = 't';
                break;
            case '\b':
                t[j] = '\\';
                j++;
                t[j] = 'b';
                break;
            default:
                t[j] = s[i];
                break;
        }
    }
    t[j] = '\0';
}

void deescape(char s[], char t[]){
    int i, j;
    for(i = j = 0; s[i] != '\0'; i++, j++){
        switch(s[i]){
            case '\\':
                i++;
                switch(s[i]){
                    case 'n':
                        t[j] = '\n';
                        break;
                    case 't':
                        t[j] = '\t';
                        break;
                    case 'b':
                        t[j] = '\b';
                        break;
                    case '\0':
                        t[j] = '\\';
                        break;
                    default:
                        t[j] = '\\';
                        j++;
                        t[j] = s[i];
                        break;
                }
                break;
            default:
                t[j] = s[i];
                break;
        }
    }
    t[j] = '\0';
}
void printstring(char s[]){
    for(int i = 0; s[i] != '\0'; i++)
        putchar(s[i]);
    putchar('\n');
}

main(){
    char s[] = "\twhat did you\njust say\\?";
    char t[MAXSIZE];
    escape(s, t);
    printstring(s);
    printstring(t);
    char t2[MAXSIZE];
    deescape(t, t2);
    printstring(t2);
}
