#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>


int fg_pid = -1; // Track current foreground(fg) pid 

/* Store relevant data for each background(bg) process */
typedef struct BgProc {
    int bg_pid;               // pid of bg process
    char** bg_args;             // args of bg process
    struct BgProc* bg_next;     // bg process following next
} BgProc;

BgProc* bg_head = NULL;         // head of bg process list 
BgProc* bg_tail = NULL;         // tail of bg process list

/* Use SIGINT signal to kill running fg child */
void int_handler(int signum) {
    (void)signum;  // suppress unused warning
    if (fg_pid != -1) {
        kill(fg_pid, SIGINT);    
        fg_pid = -1;
    }
    printf("\n");
}

/* Use SIGCHLD signal to note terminated bg child */
void kid_handler(int signum) {
    (void)signum;  // suppress unused warning

    // Set up generic read/write of BgProc linked list
    BgProc* cur = bg_head;
    BgProc* prv = NULL;

    int status;

    while (cur) {
        if (waitpid(cur->bg_pid, &status, WNOHANG)) {
            // Indicate termination of BgProc without execvp error  
            if (WIFEXITED(status) && !WEXITSTATUS(status)) {
                printf("%d: ", cur->bg_pid);
                for (int j = 0; cur->bg_args[j] != NULL; j++) {
                    printf("%s ", cur->bg_args[j]);
                }
                printf("has terminated.\n");
            }

            // Reassign prv or bg_head then free args of terminated BgProc
            if (prv) prv->bg_next = cur->bg_next;
            else bg_head = cur->bg_next;
            free(cur->bg_args);

            // Shift to next BgProc in list while maintaining pointers
            BgProc *tmp = cur;
            cur = cur->bg_next;
            free(tmp);
        } else {
            // Skip non-terminated BgProc while maintaining pointers
            prv = cur;
            cur = cur->bg_next;
        }
    }

    // Maintain tail pointer of BgProc list
    if (prv) bg_tail = prv;
    else bg_tail = bg_head;      
}

/* Use tokened getline to make arg string list */
char** arg_handler(char* cur_arg) {
    int args_size = 10; // Initial args allocation
    char** args = malloc(sizeof(char*) * args_size);
    if (!args) {
        perror("malloc");
        return NULL;
    }   

    int arg_cnt = 0;

    // Loop until the end of tokens 
    while(cur_arg != NULL) {
        // Create more list space for args if needed 
        if (arg_cnt >= args_size) {
            args_size *= 2;
            args = realloc(args, sizeof(char*) * args_size);
            if (!args) {
                perror("malloc");
                return NULL;
            } 
        }

        // Add cur_arg then increments arg_cnt 
        args[arg_cnt++] = cur_arg;
        cur_arg = strtok(NULL, "\n ");
    }

    // NULL terminate arg string list
    args[arg_cnt] = NULL;

    return args;
}

/* Print prompt consisting of user, host and directory */
void printf_prompt() {
    char *username = getlogin();
    if (!username) username = "unknown";

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) < 0) {
        perror("gethostname");
        strncpy(hostname, "unknown", sizeof(hostname));
    }

    char cur_dir[1024];
    if (!getcwd(cur_dir, sizeof(cur_dir))) {
        perror("getcwd");
        strncpy(cur_dir, "unknown", sizeof(cur_dir));
    }

    printf("%s@%s: %s > ", username, hostname, cur_dir);
    fflush(stdout);
}


/* Execute fg process, await completion then return */
void ssi_fgexec(char** args) {
    fg_pid = fork();

    if(fg_pid < 0) {
        perror("");
    }
    
    if (fg_pid == 0) {
        // In child process
        setpgid(0,0);
        execvp(args[0], args);
        perror(args[0]);
        _exit(127);
    }

    // Awaiting fg child in parent process
    waitpid(fg_pid, NULL, 0);
    fg_pid = -1;
    free(args);
}

/* Execute bg process, store process then return */
void ssi_bgexec(char** args) {
    int bg_pid = fork();

    if(bg_pid < 0) {
        perror("");
    }

    if (bg_pid == 0) {
        // In child process
        setpgid(0, 0);
        execvp(args[1], (args + 1));
        perror(args[1]);
        _exit(127);   
    }

    // Storing bg child in parent process
    BgProc *bg_node = malloc(sizeof(BgProc));
    if (!bg_node) {
        perror("malloc");
        return;
    } 
    bg_node->bg_pid = bg_pid;
    bg_node->bg_args = args; 
    bg_node->bg_next = NULL; 

    if (!bg_head) {
        bg_head = bg_node;
    } else {
        bg_tail->bg_next = bg_node;
    }
    bg_tail = bg_node;
}

/* Changes current working directory using path in args */
void ssi_chdir(char **args) {
    char* path = args[1];
    char* home = getenv("HOME");
    char* home_path = NULL;

    if (!path) {                    // User entered just cd 
        path = home; 
    } else if (path[0] == '~') {    // User entered ~ or ~/subdir
        home_path = malloc(strlen(home) + strlen(path) + 1);
        if (!home_path) {
            perror("malloc");
            return;
        } 
        // Append home to path if user entered ~/subdir
        sprintf(home_path, "%s%s", home, path + 1);
        path = home_path;
    }

    if (chdir(path)) {
        perror("chdir");
    }
    
    free(args);
    free(home_path);
}

/*  Print list of all running background processes */
void ssi_bglist(char **args) {
    // Set up generic read of BgProc linked list
    BgProc *cur = bg_head;  
    int cnt = 0;

    // Loop and print the args of each BgProc  
    while (cur) {
        printf("%d: ", cur->bg_pid);

        if (cur->bg_args) {
            for (int j = 0; cur->bg_args[j] != NULL; j++) {
                printf("%s ", cur->bg_args[j]);
            }
        }

        printf("\n");
        cnt++;
        cur = cur->bg_next;
    }

    printf("Total Background jobs: %d\n", cnt);
    free(args);
}

/* Manage signal control, ssi execution and run ssi prompt loop */
int main(void) {
    // Put ssi in its own process group
    id_t shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) < 0 && errno != EACCES) {
        // May throw error during auto-grading
        // perror("setpgid");
    }

    // Give ssi process group terminal control to receive SIGINT, SIGCHLD etc...
    if (tcsetpgrp(STDIN_FILENO, shell_pgid) < 0) {
        // May throw error during auto-grading
        // perror("tcsetpgrp");
    }

    // Set up SIGINT handler so ssi processes ctrl-C instead of dying
    struct sigaction int_sa = {0};
    int_sa.sa_handler = int_handler;
    sigemptyset(&int_sa.sa_mask);
    sigaction(SIGINT, &int_sa, NULL);

    // Set up SIGCHLD handler so ssi cleans up background jobs
    struct sigaction kid_sa = {0};
    kid_sa.sa_handler = kid_handler;
    sigemptyset(&kid_sa.sa_mask);
    sigaction(SIGCHLD, &kid_sa, NULL);

    // Set up signal set that block SIGCHLD when using shared data structures
    sigset_t cur, old;
    sigemptyset(&cur);
    sigaddset(&cur, SIGCHLD);

    // Initialize char pointers 
    char* line = strdup("\n");
    char* exec_line = NULL;
    char** exec_args = NULL;

    while (1) {
        // Let out brief process output before prompt
        usleep(75000); 
        printf_prompt();

        // Allow execution while blocking signal interrupts
        sigprocmask(SIG_BLOCK, &cur, &old);

        // Accept user input or continue if error occurs
        if (getline(&line, &(size_t){0}, stdin) == -1) {
            if (feof(stdin)) {
                break;
            }
            if (errno == EINTR) {
                clearerr(stdin);     
                continue;            
            }
            continue;            
        }

        // Duplicate line to avoid overwrites and split into executable args
        exec_line = strdup(line);
        exec_args = arg_handler(
            strtok(exec_line, "\n ")
        );

        if (exec_args[0]) {
            if (strcmp(exec_args[0], "cd") == 0) {
                ssi_chdir(exec_args);
            } else if (strcmp(exec_args[0], "bg") == 0) {
                ssi_bgexec(exec_args);
            } else if (strcmp(exec_args[0], "bglist") == 0) {
                ssi_bglist(exec_args);
            } else {
                ssi_fgexec(exec_args);
            }            
        }
        // Unblock signals and allows pending signals to interrupt 
        sigprocmask(SIG_SETMASK, &old, NULL);
    }

    // Clean up
    free(line);
    free(exec_line);
    printf("\n");
    return 0;
}
