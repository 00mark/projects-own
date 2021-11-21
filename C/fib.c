#include <stdio.h>

unsigned long long fib1(int n){
    int i;
    unsigned long long first, second, res; 

    if(n <= 1)
        return 1;
    first = 1; 
    second = 1;
    for(i = 2; i <= n; i++){
        res = second + first;
        first = second;
        second = res;
    }

    return res;
}

unsigned long long fib2(int n){
    int i;
    unsigned long long first, second, res; 

    if(n <= 1)
        return 1;
    first = 1; 
    second = 1;
    for(i = 2; i <= n; i++){
        res = second + 2 * first;
        first = second;
        second = res;
    }

    return res;
}

unsigned long long fib3(int n){
    int i;
    unsigned long long first, second, res; 

    if(n == 0)
        return 0;
    if(n == 1)
        return 1;
    first = 0; 
    second = 1;
    for(i = 2; i <= n; i++){
        res = second + i * first;
        first = second;
        second = res;
    }

    return res;
}

int main(){
    int i;

    for(i = 2; i < 21; i++)
        printf("fib1(%d): %ld, fib1(%d)/fib1(%d): %llf\n", i, fib1(i), i, i - 1
                , (long double)fib1(i) / fib1(i-1));
    printf("\n");
    for(i = 2; i < 21; i++)
        printf("fib2(%d): %ld, fib2(%d)/fib2(%d): %llf\n", i, fib2(i), i, i - 1
                , (long double)fib2(i) / fib2(i-1));
    printf("\n");
    for(i = 2; i < 21; i++)
        printf("fib3(%d): %ld, fib3(%d)/fib3(%d): %llf\n", i, fib3(i), i, i - 1
                , (long double)fib3(i) / fib3(i-1));
}
