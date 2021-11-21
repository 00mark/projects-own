#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void escape(char *s, char *t){
    int i, j; 

    for(i = j = 0; i < strlen(s); i++){
        switch(s[i]){
            case '\n': t[j++] = '\\'; t[j++] = 'n'; break;
            case '\t': t[j++] = '\\'; t[j++] = 't'; break;
            case '\\': t[j++] = '\\'; t[j++] = '\\'; break;
            default : t[j++] = s[i]; break;
        }
    }
    t[j] = '\0';
}

void de_escape(char *s, char *t){
    int i, j, escaped;

    escaped = 0;
    for(i = j = 0; i < strlen(s); i++){
        if(s[i] == '\\'){
            if(escaped){
                t[j++] = '\\';
                escaped = 0;
            }else
                escaped = 1;
            continue;
        }
        switch(s[i]){
            case 'n' : t[j++] = escaped ? '\n': 'n'; escaped = 0;
                       break;
            case 't' : t[j++] = escaped ? '\t': 't'; escaped = 0;
                       break;
            default : t[j++] = s[i]; escaped = 0; break;
        }
    }
    t[j] = '\0';
}

int main(int argv, char **argc){
    char *s, *t1, *t2;

    s = "a b\nc\t\t\\\\\\n\nd";
    printf("String before escaping:\n%s\n", s);
    t1 = malloc(strlen(s) * 2);
    escape(s, t1);
    printf("Escaped string:\n%s\n", t1);
    t2 = malloc(strlen(t1));
    de_escape(t1, t2);
    printf("De-escaped string:\n%s\n", t2);
    free(t1);
    free(t2);

    return 0;
}

