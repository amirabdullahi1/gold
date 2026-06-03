#include <string.h>
#include "quote_handler.h"

// Quote handling is not required for Assn1
char* quote_handler(char *quote) {
    char *terminator = strchr(quote, '\0');
    char *next_quote = strchr(terminator + 1, '\"');
    
    if (next_quote != NULL) {
        *terminator = ' ';
        *next_quote = '\0';
        return strtok(next_quote + 1, "\n ");
    }

    return NULL;
}