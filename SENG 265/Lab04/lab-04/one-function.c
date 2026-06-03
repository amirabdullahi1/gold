#include <stdio.h>

int main()
{
    int a;
    int b;
    int *pp;

    a = 10;
    b = 20;
	/*should print 10 and 20*/
    printf("%d %d\n", a, b);

	/*pp is a variable that holds the address to a */
    pp = &a;
	/*the value pounted to by pp = 333 (dereferencing)*/
    *pp = 333;
	
	/*should print 333 and 20*/
    printf("%d %d\n", a, b);

	/*pp is a variable that holds the address to b*/
    pp = &b;
    a = 444;
    b = 555;

	/*should print 444 and 555*/
    printf("%d %d\n", a, b);
	/*should print 555*/
    printf("%d\n", *pp);
	/*address to b*/
    printf("%p\n", (void *)pp);

/* Output
 * 10 20
 * 333 20
 * 444 555
 * 555
 *
 */

}
