#include <stdio.h>
#include <string.h>

#define MAXSTRING 50

void reverse(char s[]){
    int i, j;
    char temp;

    for(i = 0, j = strlen(s) - 1; i < j; i++, j--){
        temp = s[i]; 
        s[i] = s[j];
        s[j] = temp;
    }
}

void itoa(int n, char s[], int min_width){
    int i, sign;

    if((sign = n) < 0)
        n = -n;
    i = 0;
    do{
        s[i++] = '0' + (n % 10);
    }while((n /= 10) > 0);
    if(sign < 0)
        s[i++] = '-';
    for(; i < min_width; i++)
        s[i] = ' ';
    s[i] = '\0'; 
    reverse(s);
}

void set_arr_to_zeros(char arr[]){
    int i;

    for(i = 0; i < strlen(arr); i++)
        arr[i] = 0;
}

int main(){
    int num; 
    char res[MAXSTRING];

    num = 1341231218;
    printf("number: %d\n", num);
    itoa(num, res, 5);
    printf("itoa(number), min_width = 5: %s\n", res);
    set_arr_to_zeros(res);
    itoa(num, res, 10);
    printf("itoa(number), min_width = 10: %s\n", res);
    set_arr_to_zeros(res);
    itoa(num, res, 30);
    printf("itoa(number), min_width = 30: %s\n", res);
    set_arr_to_zeros(res);
    itoa(num, res, 40);
    printf("itoa(number), min_width = 40: %s\n", res);

    return 0;
}

