#include <stdio.h>

int binsearch(int x, int v[], int n){
    int low, high, mid;
    low = 0;
    high = n-1;
    mid = (low+high) /2;
    while(low <= high && v[mid] != x){
        if(x < v[mid])
            high = mid-1;
        else
            low = mid+1;
        mid = (low+high) /2;
    }
    return x == v[mid] ? mid : -1;
}

main(){
    int test[] = {2, 4, 6, 8, 11, 141, 200};
    printf("%d\n", binsearch(3, test, 7));
}
