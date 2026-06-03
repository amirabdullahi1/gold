#define _POSIX_C_SOURCE 202809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>

pid_t child_pid[2] = {-1, -1};

void int_handler(int signum)
{
    if (child_pid[0] != -1)
    {
        kill(child_pid[0], SIGINT);    
        child_pid[0] = -1;
    }

    if (child_pid[1] != -1)
    {
        kill(child_pid[1], SIGINT);    
        child_pid[1] = -1;
    }
}

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(0);
    }

    signal(SIGINT, int_handler);


    FILE *fp = fopen(argv[1], "w");
    if (fp == NULL)
    {
        perror("An error occured opeining the file");
        exit(1);
    }
    
    time_t start_time = time(NULL);

    //pid_t pid = -1;

    for (int idx = 0; idx < 2; ++idx)
    {
        child_pid[idx] = fork();
        if (!child_pid[idx])
        {
            if (idx == 0)
                execlp("sleep", "sleep", "5", NULL);
            else
                execlp("sleep", "sleep", "2", NULL);
        }
    }
    
    for (int idx = 0; idx < 2; ++idx)
    {
        pid_t done_pid = -1;
        done_pid = wait(NULL);
        char *name = (done_pid == child_pid[0]) ? "First" : "Second";
        if (done_pid == child_pid[0])
        {
            child_pid[0] = -1;
        } else
        {
            child_pid[1] = -1;
        }

        /* will get the number of seconds the command took */
        time_t fin_time = time(NULL);
        double elapsed_time = difftime(fin_time, start_time);

        fprintf(fp, "%s exection time: %lf\n", name, elapsed_time);
    }
    
    fclose(fp);

    return 0;

}
