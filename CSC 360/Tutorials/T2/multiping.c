#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{

    if (argc < 3)
    {
        printf("Usage: %s address [address...]\n", argv[0]);
        exit(1);
    }

    int cur_addr = 1; /* note first valid addr index is 1 */
    int pid = -1;

    /* create one process per input argument */
    while (cur_addr < argc)
    {
        pid = fork();
        if (!pid)
            break;
        cur_addr++; 
    }

    /* map each process to its input argument and start pinging */
    if (!pid)
    {
        execlp("ping", "ping", argv[cur_addr], NULL);
    }

    /* when to terminate? */
    sleep(5);
    return 0;

}
