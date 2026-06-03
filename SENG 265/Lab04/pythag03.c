#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    double a;
    double b;
    double c;

    if (argc < 3) {
        printf("usage: %s <length> <length>\n", argv[0]);
        exit(1);
    }

    a = atof(argv[1]);
    b = atof(argv[2]);

    c = pythag(a, b);

    printf("Right triangle with sides %.2f and %.2f has "
        "hypotenuse of length %.2f\n", a, b, c);
}
