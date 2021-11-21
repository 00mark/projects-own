#include <stdio.h>
#define INDEX(X) [X-1]

int main()
{
    int arr[] = {1, 2, 3, 4, 5};
    printf("%d\n", arr INDEX(2));
}
