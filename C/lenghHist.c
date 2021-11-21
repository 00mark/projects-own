#include <stdio.h>

int main(){
    int width = 10000;
    int max_heigth = 20;
    long word_length[width];
    long c, max_occurances, current_length, print_step;
    max_occurances = current_length = 0;

    for(int i = 0; i < width; i++)
        word_length[i] = 0;

    while((c = getchar()) != EOF){
        if(c == ' ' || c == '\t' || c == '\n' || c == '/' || c == ',' ||
                c == '.' || c == ':' || c == ';' || c == '\'' || c == '"' ||
                c == '?' || c == '!' || c == '=' || c =='+' || c == '|' ||
                c == '@'){
            if(current_length > 0){
                max_occurances >= ++word_length[current_length-1] ? : 
                    max_occurances++;
                current_length = 0;
            }
        }
        else
            current_length++;
    }

    print_step = (max_occurances / max_heigth) + 1;

    int maxw, longest_word;
    double num_words, total_length;;
    num_words = total_length = 0.0;
    maxw = longest_word = 0;
    for(int i = 0; i < width; i++){
        if(word_length[i] >= print_step*(1.0/8.0))
            maxw = i+1;
        if(word_length[i] > 0)
            longest_word = i+1;
        num_words += (word_length[i] / 1.0);
        total_length += (word_length[i] * (i+1) / 1.0);
    }

    width = maxw;

for(long occurances = print_step > 1 ? print_step * max_heigth : 
            max_occurances; occurances > 0; occurances -= print_step){
        printf("|");
        for(int length = 0; length < width; length++){
            if(word_length[length] >= occurances)
                printf(" █ ");
            else if(word_length[length] >= occurances - print_step*(1.0/8.0))
                printf(" ▇ ");
            else if(word_length[length] >= occurances - print_step*(1.0/4.0))
                printf(" ▆ ");
            else if(word_length[length] >= occurances - print_step*(3.0/8.0))
                printf(" ▅ ");
            else if(word_length[length] >= occurances - print_step*(1.0/2.0))
                printf(" ▄ ");
            else if(word_length[length] >= occurances - print_step*(5.0/8.0))
                printf(" ▃ ");
            else if(word_length[length] >= occurances - print_step*(3.0/4.0))
                printf(" ▂ ");
            else if(word_length[length] >= occurances - print_step*(7.0/8.0))
                printf(" ▁ ");
            else
                printf("   ");
        }
        printf("[%d]\n", occurances);
    }
    printf("+--");
    for(long i = 1; i < width; i++)
        printf("---");
    printf("-\n|");
    for(long i = 0; i < width; i++)
        i < 9 ? printf(" %d ", i+1) : printf(" %d", i+1);
    printf("\n");
    printf(" [Max Length : %d]\t[Avg Length : %.2f]\n", longest_word, 
            total_length / num_words);

}
