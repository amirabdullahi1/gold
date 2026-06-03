
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <ctype.h>


int get_date(char *st);
int get_date_sscanf(char *st);
int get_date_dtend(char *st);


int main(int argc, char *argv[]){

        char *st1 = "asdrtgkl knbvhgfk2021udfhg (blgfdl) fdgllk";
        char *st2 = "DTEND:20210102T123000";
        char *st3 = "RRULE:FREQ=WEEKLY;WKST=MO;UNTIL=20210301T235959;BYDAY=SA";

        int num1 = get_date(st1);
        printf("number found. %d\n", num1);
        int num2 = get_date_sscanf(st1);
        printf("number found. %d\n", num2);
        int num3 = get_date_dtend(st1);
        printf("number found. %d\n", num3);
        printf("-------\n");

        num1 = get_date(st2);
        printf("number found. %d\n", num1);
        num2 = get_date_sscanf(st2);
        printf("number found. %d\n", num2);
        num3 = get_date_dtend(st2);
        printf("number found. %d\n", num3);
        printf("-------\n");

        num1 = get_date(st3);
        printf("number found. %d\n", num1);
        num2 = get_date_sscanf(st3);
        printf("number found. %d\n", num2);
        num3 = get_date_dtend(st3);
        printf("number found. %d\n", num3);
}

// COMPLETE IMPLEMENTATION OF FUNCTIONS BELOW.

int get_date(char *st){
        // Indexes through st, char by char, until it
        // encounters the first digit, and proceeds from there.

}

int get_date_sscanf(char *st){
        // Uses the sscanf function to find date in st.

}

int get_date_dtend(char *st) {
        // Use direct memory access and pointer arithmetic to find the date in st

}
