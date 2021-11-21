#include <stdio.h>

double computeCel(double fahr){
    return (5.0/9.0) * (fahr-32.0);
}

main(){
    for(int i = 0; i <= 300; i += 20){
        printf("%d\t%.1f\n", i, computeCel(i));
    }
}
