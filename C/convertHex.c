#include <stdio.h>
#define MAXCHARS 30

char s[MAXCHARS];

unsigned long htoi(char s[]){
    unsigned long current;
    unsigned long i = 0, j = 0, dec = 0;
    if(s[0] == '\0')
        return -1;
    if(s[0] == '0'){
        if(s[1] == 'x' || s[1] == 'X')
            i = 2;
        else if(s[1] == '\0')
            return 0;
        else
            return -1;
    }
    for(; s[j+i] != '\0'; j++);
    for(; j > 0; j--, i++){
        if(s[i] < '0' || (s[i] > '9' && s[i] < 'A') || (s[i] > 'F' &&
                    s[i] < 'a') || s[i] > 'f')
            return -1;
        current = 1;
        for(int n = j-1; n > 0; n--)
            current *= 16;
        if(s[i] <= '9')
            dec += current * (s[i] - '0');
        else if(s[i] <= 'F')
            dec += current * (s[i] - 'A' + 10);
        else
            dec += current * (s[i] - 'a' + 10);
    }
    return dec;
}

void readhex(){
    int i, c;
    for(i = 0; i < MAXCHARS-1 && (c = getchar()) != '\n' && c != EOF; ++i)
        s[i] = c;
    s[i] = '\0';
}

main(){
    printf("Enter Hex (optional 0x/0X prefix)\n> ");
    readhex();
    printf("Hex : %s, Dec : %lu\n",s, htoi(s));
}
