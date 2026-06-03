#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 10

void rotate(int shift, int *nums);


 int main(){
 	 

         int i = 0;
         char *sep = "";
         // declare and initialize an int array arr_1 of size MAX_SIZE and initialize it with random values
         // For example, declare and initialize arr_1 to {1,2,3,4,5,6,7,8,9,10}
         int arr_1[MAX_SIZE] = {1,2,3,4,5,6,7,8,9,10};

         // This for statement prints the content of arr_1 to console
         for (i = 0; i < MAX_SIZE; i++) {
             printf("%s%d", sep, arr_1[i]);
             sep = ", ";
         }
         printf("\n");

	 rotate(1, arr_1);

	 sep = "";

         for (i = 0; i < MAX_SIZE; i++) {
             printf("%s%d", sep, arr_1[i]);
             sep = ", ";
         }
         printf("\n");
         // Outcome on console from the above for loop should be 2, 3, 4, 5, 6, 7, 8, 9, 10, 1

	 sep = "";

         rotate(2,arr_1);

         for (i = 0; i < MAX_SIZE; i++) {
              printf("%s%d", sep, arr_1[i]);
              sep = ", ";
         }
         printf("\n");
         // Outcome on console frvoid rotate(int shift, int *nums) om the above for loop should be 4, 5, 6, 7, 8, 9, 10, 1, 2, 3

 }


 /*
  * Design a function rotate(int num, .........) that take a number as a parameter (int num) and any other parameters as required
  * When called, the function will modify arr_1 by moving each of its values to an index lowered by num. If the new index becomes negative, then MAX_SIZE is added to get the correct index
  * For example, if num = 2, then a number at index 4 is moved to index 2 (4 - num) and a number at index 1 is moved to index 9(1 - num + MAX_SIZE)
  * Another example, if at the start, arr_1 is {2,4,6,8,1,5},
  * calling arr_1 with num = 2 should change arr_1 to {6,8,1,5,2,4}
  */


 void rotate(int shift, int nums[]) {
	int temps[MAX_SIZE];
	int relocate;
	for(int i=0; i<MAX_SIZE; i++){
	 	relocate = i-shift;
	        if(relocate<0)
        	relocate += MAX_SIZE;
	/*printf("%d relocate ", relocate);*/	
		temps[relocate] = nums[i];
	/*printf("%d num in temp \n", temps[relocate]);*/
	}

	for (int i = 0; i < MAX_SIZE; i++) {
              nums[i]=temps[i];
         }
 }

