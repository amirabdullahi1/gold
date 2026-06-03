#include <math.h>
#include <stdio.h>

double pythag(double a, double b)
{
    return sqrt(a * a + b * b);
}


int main()
{
    printf("Right triangle with sides %.2f and %.2f has "
        "hypotenuse of length %.2f\n",
        10.0, 13.0, pythag(10, 13));

    printf("Right triangle with sides %.2f and %.2f has "
        "hypotenuse of length %.2f\n",
        21.9, 31.2, pythag(21.9, 31.2));

    printf("Right triangle with sides %.2f and %.2f has "
        "hypotenuse of length %.2f\n",
        719.21, 21.2, pythag(719.21, 21.2));
}
