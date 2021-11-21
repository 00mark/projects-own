#include <stdio.h>

int get_volume(int vals[], int len){
    int i, j, vol, cur_vol, cur_height, last_height, zeroed;

    if(len <= 2)
        return 0;
    vol = cur_vol = zeroed = 0;
    cur_height = vals[0];
    for(i = 1; i < len; i++){
        if(i == len - 1 && vals[i] < cur_height){
            last_height = vals[i];
            for(j = len - 2; j >= 0 && vals[j] != cur_height; j--){
                if(vals[j] >= last_height)
                    last_height = vals[j];
                cur_vol -= (cur_height - last_height);
            }
            vol += cur_vol;
        }else if(vals[i] == 0)
            zeroed = 1;
        else if(vals[i] >= cur_height){
            vol += zeroed ? 0 : cur_vol;
            cur_vol = zeroed = 0;
            cur_height = vals[i];
        }else if(vals[i] < cur_height)
            cur_vol += cur_height - vals[i];
    }

    return vol;
}

void draw_hist(int vals[], int len){
    int i, j, max, spaces, tmp;

    max = vals[0];
    for(i = 1; i < len; i++){
        if(vals[i] > max)
            max = vals[i];
    }
    spaces = 0;
    tmp = max;
    while(tmp > 0){
        tmp /= 10;
        spaces++;
    }
    for(i = max; i > 0; i--){
        printf("[%*d] |", spaces, i);
        for(j = 0; j < len; j++){
            if(vals[j] >= i)
                printf("█ ");
            else
                printf("  ");
        }
        printf("\n");
    }
    printf("-");
    for(;spaces > 0; spaces--)
        printf("-");
    printf("--+");
    for(i = 0; i < len; i++)
        printf("--");
}

int main(){
    int vals[] = {7 ,4 , 5, 11, 12, 7, 9, 1, 0, 13};
    int i, len;

    len = sizeof(vals) / sizeof(int);
    draw_hist(vals, len);
    printf("\n[Volume contained: %d]\n", get_volume(vals, len));
}
