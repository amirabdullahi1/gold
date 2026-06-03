#include <stdio.h>

void rotate(int *m, int *n, int *p, int *q)
{
    int temp;
    temp = *m;

    *m = *n;
    *n = *p;
    *p = *q;
    *q = temp;
}


int main(){
    int a = 111;
    int b = 222;
    int c = 333;
    int d = 444;

    printf("%d %d %d %d\n", a, b, c, d);

    rotate(&a, &b, &c, &d);
    printf("%d %d %d %d\n", a, b, c, d);

    rotate(&a, &b, &c, &d);
    printf("%d %d %d %d\n", a, b, c, d);
}
