
/*
This program when run, should take 2 command line arguments, both will be integers.
1) First command line integer argument. Allocated an array of size of this number. The array
whill hold integers. Further instruction provided in the code below.
2) Second command line integer argument. This will be the seed provided to the
random number generator.
*/


#include <stdio.h>
#include <stdlib.h>

int compare_ints(const void *m, const void *n)
{
    int im = *(int *)m;
    int in = *(int *)n;

    return im - in;
}



int main(int argc, char *argv[])
{

    int i, num_ints;

    if (argc < 3) {
        fprintf(stderr, "usage: %s <num ints> <seed>\n", argv[0]);
        exit(1);
    }

    num_ints = atoi(argv[1]); // First command line argument
    srand(atoi(argv[2])); // Seeds the random number generator

	int *nums  = malloc(num_ints*(sizeof(int))); 
	
	if (nums == NULL){
			printf("malloc error");
			return 0; 
       	}

    // For loop generated random numbers
    for (i = 0; i < num_ints; i++) {
       	int cur_num = rand();
		nums[i]= cur_num;
		printf("%d\n", cur_num); 
    }
	
    printf("sorted array \n"); 

	qsort(nums, num_ints, sizeof(int),compare_ints);
	
	for (i = 0; i < num_ints; i++) {
        printf("%d\n", nums[i]); 
    } 

	free(nums); 

/*  Create an array of integers of the size of the first command line argument
    Fill this array with random numbers
    Sort the array
    Print the results of the array
    FREE the array at the end.
    You will need to use dynamic memory
*/

	return 0;
}

