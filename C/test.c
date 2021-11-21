#include <stdio.h>

double recur(double x, double q, double b, int num){
    if(num == 0)
        return x;
    return (q * recur(x, q, b, num - 1)) / (1 + b * recur(x, q, b, num - 1));
}

int main(){
    int i;

    for(i = 0; i < 20; i++)
        printf("%lf\n", recur(0.00027, 16, 0.001, i));
}
