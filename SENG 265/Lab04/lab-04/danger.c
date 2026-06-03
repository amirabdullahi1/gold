#include <stdio.h>

#define MAX_NUMS  5

void rogue(int *numbers, int num)
{
    int i;

    for (i = -2; i <= num; i++) {
        numbers[i] = 111 * i + 11;
    }
}


int main() {
    int innocent = 123;
    int nums[MAX_NUMS] = {1, 2, 3, 4, 5};
    int really_innocent = 456;
    int i;

    printf("%d %d\n", innocent, really_innocent);
    for (i = 0; i < MAX_NUMS; i++) {
        printf("%d: %d\n", i, nums[i]);
    }

    rogue(nums, MAX_NUMS);

    printf("----------------\n");
    printf("%d %d\n", innocent, really_innocent);
    for (i = 0; i < MAX_NUMS; i++) {
        printf("%d: %d\n", i, nums[i]);
    }
/*number[-2] is outsiode of availbale memory produces seg fault 
 *therefore does not reach printf("--------")
 */

}
