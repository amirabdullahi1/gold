#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 * * Compile-time constants
 * */

#define MAX_EXCL 23
#define MAX_IDX 100
#define MAX_WRD_LEN 20
#define MAX_LINE 100
#define MAX_LINE_LEN 70

char excl[MAX_EXCL][MAX_WRD_LEN];
char *idx[MAX_IDX];
char lines[MAX_LINE][MAX_LINE_LEN];
char output[MAX_LINE][MAX_LINE_LEN];

int NUM_EXCL=0;
int NUM_IDX=0;
int NUM_LINES=0;

int contains( char *wrd);
void add(char *wrd);
void print_line(char *line, char *wrd);
void string_toupper(char *wrd, int wrd_len);
void string_tolower(char *wrd, int wrd_len);

int main(int argc, char *argv[]) {
        //stores exclusion words
        for(; !strstr(excl[NUM_EXCL-1], "\"") ; NUM_EXCL++) {
                fgets(excl[NUM_EXCL], 20,  stdin);
        }

        //stores lines for indexing
        while( fgets( lines[NUM_LINES], 70, stdin)!=NULL ) {
                NUM_LINES++;
        }

        for(int i=0; i<NUM_LINES; i++) {
                //creates copy of lines unaffected by strtok
                strncpy (output[i], lines[i], strlen(lines[i]));
                //stores index words from lines[i]
                for(char* cur_word =strtok( lines[i], " \n"); cur_word!=NULL; cur_word = strtok(NULL, " \n")) {
                  if(contains(cur_word)==0)
                          add(cur_word);
        }
        }

        //runs through each index word looking for it in all lines-for-indexing
        for(int i = 0; i<NUM_IDX; i++) {
                for(int line = 0; line<NUM_LINES; line++) {

                        char* idx_loc= strstr(output[line],idx[i]);
                        if(idx_loc!=NULL && (!isalpha(idx_loc[-1]) && !isalpha(strlen(idx[i])))) {
                                print_line(output[line],idx_loc);
                        }
                }
        }

        return 0;
}

//checks that wrd is not an exclusion word or duplicate index word
int contains( char *wrd) {
        for(int i = 1; i<NUM_EXCL-1; i++) {
                char* wrd_loc = strstr (excl[i], wrd);
                if( wrd_loc!=NULL && !isalpha(wrd_loc[-1]) && !isalpha(wrd_loc[strlen(wrd)]) )
                        return 1;
        }

        for(int i = 0; i<NUM_IDX; i++) {
                char* wrd_loc = strstr (idx[i], wrd);
                if( wrd_loc!=NULL && !isalpha(wrd_loc[-1]) && !isalpha(wrd_loc[strlen(wrd)]) )
                        return 1;
        }
        return 0;
}

//stores index words
void add(char *wrd) {
        char* tmp;
        for(int i = 0; i<MAX_IDX; i++) {
                //adds wrd
                if(idx[i]==NULL) {
                        idx[i] = wrd;
                        NUM_IDX++;
                        return;
                }
                //placess wrd alphabettically if idx[i] is not empty
                else if(strncmp( wrd, idx[i], MAX_WRD_LEN)<0) {
                        tmp = idx[i];
                        idx[i]=wrd;
                        wrd = tmp;
                }
        }
}

//capitalizes each word in string
void string_toupper(char *wrd, int wrd_len) {
        for(int i =0; i<wrd_len; i++) {
                wrd[i]=toupper(wrd[i]);
        }

}

//returns string to all lowercase form
void string_tolower(char *wrd, int wrd_len) {
        for(int i =0; i<wrd_len; i++) {
                wrd[i]=tolower(wrd[i]);
        }
}

//prints line(s) with corresponding index word(s) capitalized
void print_line(char *line, char *wrd) {
        //determines length of word to be capitalized
        int wrd_len = 0;
        while( isspace(wrd[wrd_len])==0 ) {
                wrd_len++;
        }

        string_toupper(wrd,wrd_len);
        fprintf(stdout,"%s", line);
        string_tolower(wrd, wrd_len);
}