#include <stdio.h>
#define INTEGER_SIZE 32

int get_fill_value(int p, int n){
    int fill_value1, fill_value2;
    fill_value1 = ~0 << (p+1);
    fill_value2 = 0;
    if(p+1 > n){ 
        for(int spot = p+1-n; spot > 0; spot--){
            int tmp = 1;     
            for(int exp = spot-1; exp > 0; exp--)
                tmp *= 2;
            fill_value2 += tmp;
        }
    }
    return fill_value1 + fill_value2;
}

unsigned setbits(unsigned x, int p, int n, unsigned y){
    int fill_value = get_fill_value(p, n);
    return (x & fill_value) | ((y << p+1-n) & ~fill_value);
}

unsigned invert(unsigned x, int p, int n){
    int fill_value = get_fill_value(p, n);
    return (x & fill_value) | ~(x | fill_value);
}

unsigned rightrot(unsigned x, int n){
    int bits[INTEGER_SIZE]; 
    for(int i = INTEGER_SIZE; i > 0; i--){
        unsigned current = 1;
        for(int j = i-1; j > 0; j--)
            current *= 2;        
        if(current <= x){
            x -= current;
            bits[INTEGER_SIZE-i] = 1;
        }else
            bits[INTEGER_SIZE-i] = 0;
    }
    int cur, prev;
    for(; n > 0; n--){
        prev = -1;
        for(int i = 0; i < INTEGER_SIZE; i++){
            cur = bits[i];
            bits[i] = prev;
            prev = cur; 
        }
        bits[0] = prev;
    }
    unsigned result = 0;
    for(int i = INTEGER_SIZE; i > 0; i--){
        if(bits[INTEGER_SIZE-i] == 1){
            unsigned current = 1;
            for(int j = i-1; j > 0; j--)
                current *= 2;
            result += current;
        }
    }
    return result;
}

int bitcount(unsigned x){
    int b = 0;
    while(x != 0){
        x &= (x-1);
        b++;
    }
    return b;
}

void printbinary(unsigned x){
    if(x == 0)
        putchar('0');
    else{
        int max_exp = 0;
        unsigned tmp = 1;
        while(tmp <= x && max_exp < INTEGER_SIZE){
            max_exp++;
            tmp *= 2;
        }
        for(; max_exp > 0; max_exp--){
            unsigned current = 1;
            for(int j = max_exp-1; j > 0; j--)
                current *= 2;
            if(current <= x){
                x -= current;
                putchar('1');
            }else
                putchar('0');
        }
    }
    printf("\n");
}

main(){
    unsigned x = ~0;
    printbinary(x);
    int x2 = ~0 << 31;
    printf("%d\n", bitcount(x));
    printf("%d\n", x2);
}
