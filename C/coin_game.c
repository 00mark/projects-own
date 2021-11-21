#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdio_ext.h>

long RANDOM_HALF = (1 << 30) - 1;

enum {HEADS, TAILS};

void die(const char *msg)
{
    printf("%s\n", msg);
    exit(-1);
}

struct board{
    int x;
    int y;
    char **fields;
};

void print_board(struct board *board)
{
    int i, j;
    char border[board->y * 2];

    memset(border, '-', board->y * 2 - 1);
    border[board->y * 2 - 1] = '\0';
    printf("+%s+\n", border);
    for(i = 0; i < board->x; i++){
        printf("|");
        for(j = 0; j < board->y; j++){
            if(j != board->y - 1)
                printf("%s ", board->fields[i][j] == HEADS ? "●" : "○");
            else
                printf("%s", board->fields[i][j] == HEADS ? "●" : "○");
        }
        printf("|\n");
    }
    printf("+%s+\n", border);
}

struct board *init_board(int x, int y)
{
    int i, j;
    struct board *board;
    board = malloc(sizeof(struct board));
    if(!board)
        die("cannot malloc");
    board->x = x;
    board->y = y;
    board->fields = malloc(sizeof(char * ) * x);
    if(!board->fields)
        die("cannot malloc");
    for(i = 0; i < x; i++){
        board->fields[i] = malloc(y);
        if(!board->fields[i])
            die("cannot malloc");
    }
    
    for(i = 0; i < x; i++)
        for(j = 0; j < y; j++)
            board->fields[i][j] = random() > RANDOM_HALF ? HEADS : TAILS;

    return board;
}

int calc_key_location(struct board *board)
{
    int i, sum, val;
    char key_location_b[6];

    for(sum = i = 0; i < 8; i++){
        sum += board->fields[i][1] == HEADS ? 1 : 0;
        sum += board->fields[i][3] == HEADS ? 1 : 0;
        sum += board->fields[i][5] == HEADS ? 1 : 0;
        sum += board->fields[i][7] == HEADS ? 1 : 0;
    }
    key_location_b[5] = sum & 1; 

    for(sum = i = 0; i < 8; i++){
        sum += board->fields[i][2] == HEADS ? 1 : 0;
        sum += board->fields[i][3] == HEADS ? 1 : 0;
        sum += board->fields[i][6] == HEADS ? 1 : 0;
        sum += board->fields[i][7] == HEADS ? 1 : 0;
    }
    key_location_b[4] = sum & 1;

    for(sum = i = 0; i < 8; i++){
        sum += board->fields[i][4] == HEADS ? 1 : 0;
        sum += board->fields[i][5] == HEADS ? 1 : 0;
        sum += board->fields[i][6] == HEADS ? 1 : 0;
        sum += board->fields[i][7] == HEADS ? 1 : 0;
    }
    key_location_b[3] = sum & 1;

    for(sum = i = 0; i < 8; i++){
        sum += board->fields[1][i] == HEADS ? 1 : 0;
        sum += board->fields[3][i] == HEADS ? 1 : 0;
        sum += board->fields[5][i] == HEADS ? 1 : 0;
        sum += board->fields[7][i] == HEADS ? 1 : 0;
    }
    key_location_b[2] = sum & 1; 

    for(sum = i = 0; i < 8; i++){
        sum += board->fields[3][i] == HEADS ? 1 : 0;
        sum += board->fields[4][i] == HEADS ? 1 : 0;
        sum += board->fields[6][i] == HEADS ? 1 : 0;
        sum += board->fields[7][i] == HEADS ? 1 : 0;
    }
    key_location_b[1] = sum & 1;

    for(sum = i = 0; i < 8; i++){
        sum += board->fields[4][i] == HEADS ? 1 : 0;
        sum += board->fields[5][i] == HEADS ? 1 : 0;
        sum += board->fields[6][i] == HEADS ? 1 : 0;
        sum += board->fields[7][i] == HEADS ? 1 : 0;
    }
    key_location_b[0] = sum & 1;

    for(sum = i = 0, val = 1; i < 6; i++){
        sum += key_location_b[5 - i] * val;
        val *= 2;
    }
    
    return sum;
}

int calc_coin_flip(struct board *board, int key_location)
{
    int i, mask, x, y;
    char key_location_diffs_b[6], key_location_cur;

    key_location_cur = calc_key_location(board);
    mask = 1;
    for(i = 0; i < 6; i++){
        key_location_diffs_b[5 - i] = ((key_location & mask) ^ (key_location_cur
                & mask)) >> i;
        mask = mask << 1;
    }
    if(key_location_diffs_b[0]){
        if(key_location_diffs_b[1]){
            if(key_location_diffs_b[2]){
                x = 7;
            }else{
                x = 6;
            }
        }else{
            if(key_location_diffs_b[2]){
                x = 5;
            }else{
                x = 4;
            }
        }
    }else{
        if(key_location_diffs_b[1]){
            if(key_location_diffs_b[2]){
                x = 3;
            }else{
                x = 2;
            }
        }else{
            if(key_location_diffs_b[2]){
                x = 1;
            }else{
                x = 0;
            }
        }
    }

    if(key_location_diffs_b[3]){
        if(key_location_diffs_b[4]){
            if(key_location_diffs_b[5]){
                y = 7;
            }else{
                y = 6;
            }
        }else{
            if(key_location_diffs_b[5]){
                y = 5;
            }else{
                y = 4;
            }
        }
    }else{
        if(key_location_diffs_b[4]){
            if(key_location_diffs_b[5]){
                y = 3;
            }else{
                y = 2;
            }
        }else{
            if(key_location_diffs_b[5]){
                y = 1;
            }else{
                y = 0;
            }
        }
    }

    return x * 8 + y;
}

void free_board(struct board *board)
{
    int i;

    for(i = 0; i < board->x; i++)
        free(board->fields[i]);
    free(board->fields);
    free(board);
}

int main()
{
    int c, key_location, chosen_coin;
    struct board *board;

    c = chosen_coin = key_location = 0;
    while(1){
        printf("Play as player [1] or [2]?\n");
        c = getchar();
        if(c == '1' || c =='2')
            break;
        __fpurge(stdin);
        printf("Invalid input!\n");
    }
    srand(time(NULL));
    board = init_board(8, 8);
    key_location = random() % 64;
    switch(c){
        case '1': print_board(board);
                  printf("The key was placed under coin %d\n", key_location);
                  printf("Which coin should be flipped?\n");
                  while(scanf("%d", &chosen_coin) < 1 || chosen_coin < 0 || 
                          chosen_coin > 63){
                      printf("Invalid input!\n");
                      __fpurge(stdin);
                  }
                  board->fields[chosen_coin / 8][chosen_coin % 8] = 
                      (board->fields[chosen_coin / 8][chosen_coin % 8] == HEADS
                       ? TAILS : HEADS);
                  print_board(board);
                  chosen_coin = calc_key_location(board);
                  printf("The computer guesses the key to be under coin %d\n",
                          chosen_coin);
                  break;
        case '2': chosen_coin = calc_coin_flip(board, key_location); 
                  board->fields[chosen_coin / 8][chosen_coin % 8] = 
                      (board->fields[chosen_coin / 8][chosen_coin % 8] == HEADS
                       ? TAILS : HEADS);
                  print_board(board);
                  printf("Under which coin is the key?\n");
                  while(scanf("%d", &chosen_coin) < 1 || chosen_coin < 0 || 
                          chosen_coin > 63){
                      printf("Invalid input!\n");
                      __fpurge(stdin);
                  }
                  break;
    }

    printf("%s\n", key_location == chosen_coin ? "SUCCESS" :
          "FAILURE");
    printf("chosen coin = %d, key location = %d\n", chosen_coin, key_location);
    free_board(board);
    return 0;
}
