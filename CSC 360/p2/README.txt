CSc 360: Operating Systems — Fall 2025
Programming Assignment 2 (P2): A Multi Thread Simulator (MTS)

Name: Amir Abdullahi
Netlink ID: amirabdullahi1
Student ID: V00977658

Features Implemented
--------------------
I have implemented the MTS simulation according to the project specification. The simulation includes:
- Train threads representing inbound/outbound trains.
- Queue management using pthread_mutex_t and pthread_cond_t for safe concurrent access.
- Single-track logic enforcing one train on the main track at a time.
- Load-time ordering for trains with the same loading time to preserve input file order.
- Logging of train events to output.txt with timestamps relative to simulation start.
- Track starvation prevention (no more than 2 consecutive trains in the same direction if opposite trains are waiting).
- FIFO queues for inbound and outbound trains.
- Configurable load and crossing times per train from the input file.
- Proper synchronization for thread-safe operations.

Compilation
-----------
Use the provided Makefile:

    make

It compiles with -pthread and -lm to support threads and math functions.

Execution
---------
Run the simulation as follows:

    ./mts <input.txt>

Example:

    ./mts input.txt

Estimated Execution Time
------------------------
For a test input file with 80 trains, the simulation completed in approximately:

    ~45 seconds

Note: The actual execution time depends on the load_time and Xing_time values specified in input.txt.

Output
------
The simulation writes logs to output.txt:

    hh:mm:ss.d Train XX is ready to go West/East
    hh:mm:ss.d Train XX is ON the main track going West/East
    hh:mm:ss.d Train XX is OFF the main track after going West/East

Notes
-----
- The simulation enforces single-track safety with condition variables.
- Stack size per thread is set safely at PTHREAD_STACK_MIN (128 KB).
- All dynamically allocated memory is freed at the end of the simulation.

Bonus Features
--------------
- None implemented.
