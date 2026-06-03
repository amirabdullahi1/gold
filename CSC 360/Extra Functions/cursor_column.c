#include <stdio.h>
#include <unistd.h>
#include <termios.h>

//  Cursor column is not required for Assn1
int cursor_column() {
    struct termios oldt, newt;

    // Switch terminal to raw mode
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Ask terminal for cursor position
    printf("\033[6n");

    // Parse terminal response for cursor column
    int col;
    if (scanf("\033[%*d;%dR", &col) != 1) {
        perror("");
    }
    fflush(stdout);


    // Restore terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return col;
}
