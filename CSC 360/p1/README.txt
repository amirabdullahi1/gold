CSc 360: Operating Systems — Fall 2025
Programming Assignment 1 (P1): A Simple Shell Interpreter (SSI)

Name: Amir Abdullahi
Netlink ID: amirabdullahi1
Student ID: V00977658


Features Implemented
--------------------

Foreground Execution
- Prompt shows `username@hostname: cwd >`.
- Executes external programs using fork() and execvp().
- Handles arbitrary arguments correctly (via arg_handler).
- Responds to Ctrl-C (SIGINT) by killing only the foreground process.
- Handles EOF (Ctrl-D) gracefully, exiting SSI.

Changing Directories (cd)
- `cd` with no argument → home directory ($HOME).
- `cd ~` → home directory.
- `cd ~/subdir` → subdir in home directory.
- Relative paths (`cd ..`, `cd ./subdir`) work correctly.
- Absolute paths (`cd /usr/bin`) work correctly.
- Prompt updates with the current working directory.

Background Execution
- `bg <cmd> <args>` runs program in the background.
- `bglist` lists active background jobs and total count.
- Background jobs terminate correctly and are removed from the linked list.
- On termination, prints "<pid>: <cmd> <args> has terminated."

Signal Handling
- SIGINT handled — interrupts foreground process but not SSI.
- SIGCHLD handled — detects and cleans up terminated background processes.
- Prompts always reappear after signal handling.


Limitations
--------------------------
- Background jobs are started with execvp(args[1], args+1) because `bg` is a built-in command.
- Ceiling on number of background processes as each needs increasing amounts of unfreed memory from strdup.
- Output from background processes may mix with SSI prompt (expected per spec).


Compilation
-----------
Compiles cleanly with:

    make

Produces executable:

    ./ssi

No warnings with -Wall -Wextra -std=c11.


Bonus Features
--------------
- None implemented.
