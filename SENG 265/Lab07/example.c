
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){

        int count = 10;

        double *num = calloc(10, sizeof(double));
        // This line is incorrect - how do we properly
        // reserve room for 10 doubles (hint: sizeof)

        // How to check if malloc fails????
		if (num == NULL){
			printf("malloc error");
			return 0; 
       	}
        // What are the values in the reserved memory?
        // Is there a way to make sure they start off at zero?
       
        for (int i = 0; i < count; i++){
                printf(" %d ----> %f\n", i, num[i]);
        }
        

        double x = 1.5;
        for (int i = 0; i < count; i++){
                num[i] = x;
                x += 1;
        }

        for (int i = 0; i < count; i++){
                printf(" %d ----> %f\n", i, num[i]);
        }

        // What do we need to do before we exit?
		free(num);
        return 0;
}
