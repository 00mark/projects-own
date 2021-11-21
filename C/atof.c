#include <string.h>
#include <stdio.h>
#include <ctype.h>

/*int isspace(char c){
    return (c == ' ' || c == '\n' || c == '\t') ? 1 : 0;
}

int isdigit(char c){
    return (c >= '0' && c <= '9') ? 1 : 0;
}
*/

double atof(char s[]){
    double val, power;
    int i, sign, exp, divide;

    exp = 0;
    for(i = 0; isspace(s[i]); i++)
        ;
    sign = (s[i] == '-') ? -1 : 1;
    if(s[i] == '+' || s[i] == '-')
        i++;
    for(val = 0.0; isdigit(s[i]); i++)
        val = 10.0 * val + (s[i] - '0');
    if(s[i] == '.')
        i++;
    for(power = 1.0; isdigit(s[i]); i++){
        val = 10.0 * val + (s[i] - '0');
        power *= 10.0;
    }
    if(i < strlen(s) - 2 && s[i++] == 'e'){
        if(s[i] == '-'){
            i++;
            divide = 1;
            for(; i < strlen(s) && isdigit(s[i]); i++)
                exp = 10 * exp + (s[i] - '0');
        }else if(s[i++] == '+'){
            divide = 0;
            for(; i < strlen(s) && isdigit(s[i]); i++)
                exp = 10 * exp + (s[i] - '0');
        }
    }    
    val = sign * val / power;
    if(exp){
        if(divide){
            while((exp--) > 0)
                val /= 10;
        }else{
            while((exp--) > 0)
                val *= 10;
        }
    }
    
    return val;
}

int main(){
    char s[] = "123.45e";

    printf("String: %s, atof(String): %f", s, atof(s));

    return 0;
}
