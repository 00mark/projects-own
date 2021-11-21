#include <stdlib.h>
#include <stdio.h>

int **get_spiral_arr(int n, int m){
    int i, j, cur, x, **vals;

    vals = malloc(sizeof(int *) * n);
    for(i = 0; i < n; i++)
        vals[i] = malloc(sizeof(int) * m);
    cur = 1;

    for(x = 0; x < (n + 1) / 2; x++){
        for(j = x, i = x; cur < n*m + 1 && j < m - x; j++)
            vals[i][j] = cur++;
        for(j = j - 1, i = x + 1; cur < n*m + 1 && i < n - x; i++)
            vals[i][j] = cur++;
        for(j = j - 1, i = i - 1; cur < n*m + 1 && j >= x; j--)
            vals[i][j] = cur++;
        for(j = x, i = n - 2 - x; cur < n*m + 1 && i > x; i--)
            vals[i][j] = cur++;
    }
    
    return vals;
}

int main(int argv, char **argc){
    int total, size, i, j, n, m, **vals;

    n = atoi(argc[1]);
    m = atoi(argc[2]);
    size = 1;
    for(total = n*m; total > 0; total /= 10)
        size++;
    vals = get_spiral_arr(n, m);
    for(i = 0; i < n; i++){
        for(j = 0; j < m; j++)
            printf("%*d", size, vals[i][j]);
        printf("\n");
    }
    for(i = 0; i < n; i++)
        free(vals[i]);
    free(vals);
}
