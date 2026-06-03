#define _XOPEN_SOURCE 600
#define NS_PER_SEC 1000000000
#define US_PER_DEC_SEC 100000

#include <time.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>

/*
 * One mutex & cond to control queue access.
 * One mutex & cond to control track access.
 */
pthread_mutex_t track_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t track_cond = PTHREAD_COND_INITIALIZER; 
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER; 
struct timespec time_0, time_n; /* Track sim time. */

FILE *output, *input; /* Input/Output.txt pointers. */

int load_ties[99] = {0}; /* Number of tied loaders. */
int res_ties[99] = {0}; /* Number of ties resolved. */

/* Hold input.txt train info & Xing info. */
typedef struct {
    int train_num, order_num, load_time, Xing_time;
    char train_dir;
    bool track_Xer, track_Xed; 
    pthread_t thread;
} train_t;

/* Hold out/in bound train_t. */
typedef struct bound_node {
    train_t *train;           
    struct bound_node *next;
} bound_node_t;

/*
 * Queue out/in bound train_t.
 * 1 for each priority of dir.
 */
bound_node_t *WEST_bound;
bound_node_t *EAST_bound;
bound_node_t *west_bound;
bound_node_t *east_bound;

/* Log output.txt train sim info. */
void log_sim (int num, char *ctx, char *dir) {
    clock_gettime(CLOCK_MONOTONIC, &time_n);
    
    double diff_raw_sec = time_n.tv_sec - time_0.tv_sec;
    double diff_raw_nsec = time_n.tv_nsec - time_0.tv_nsec;

    /* Handle if nsec diff crosses a second. */
    if (diff_raw_nsec < 0) {
        diff_raw_sec -= 1;
        diff_raw_nsec += 1000000000.0;
    }

    /* Format total time diff. */
    double time_diff = diff_raw_sec + diff_raw_nsec / NS_PER_SEC;
    int diff_h = ((int) time_diff / 3600) % 24;
    int diff_m = ((int) time_diff / 60) % 60;
    int diff_s = ((int) time_diff) % 60;
    int diff_ds = (int) fmod(time_diff * 10, 10);

    fprintf(output, "%02d:%02d:%02d.%1d Train %2d %s %4s\n", diff_h, diff_m, diff_s, diff_ds, num, ctx, dir);
}

/* WEST/EAST/west/east dequeue train as node per outbound arg. */
void inbound_dequeue (bound_node_t **inbound) {
    /* Empty case. */
    if (!*inbound) { 
        return;
    }

    /* FIFO */
    bound_node_t *curr = *inbound;      
    *inbound = curr->next;              
    curr->next = NULL;
    free(curr);
}

/* WEST/EAST/west/east enqueue train as node per outbound arg. */
void outbound_enqueue (bound_node_t **outbound, train_t *train) {
    bound_node_t *node = malloc(sizeof(bound_node_t));
    if (!node) {
        perror("malloc");
        exit(1);
    } 

    /* Link to train. */
    node->train = train;
    node->next = NULL;

    /* Empty case. */
    if (!*outbound) { 
        *outbound = node;
        return;
    }

    /* LILO */
    bound_node_t *curr = *outbound;
    while (curr->next) {
        curr = curr->next;
    }
    curr->next = node;
}

/* Controls train behavior & timing. */
void *train_routine (void *train_void) {
    train_t *train = (train_t *)train_void; /* Convert void type. */

    usleep(train->load_time * US_PER_DEC_SEC); /* "Load" train. */
    char *track_dir = (toupper(train->train_dir) == 'W') ? "West" : "East";

    /* Enqueue train with lock to avoid concurrent modification. */
    pthread_mutex_lock(&queue_mtx);
    /* Await tied-loaders read prior in input.txt to queue. */
    while(res_ties[train->load_time - 1] < train->order_num) {
        pthread_cond_wait(&queue_cond, &queue_mtx);
    }
    switch (train->train_dir) {
        case 'W': outbound_enqueue(&WEST_bound, train); break;
        case 'E': outbound_enqueue(&EAST_bound, train); break;
        case 'w': outbound_enqueue(&west_bound, train); break;
        case 'e': outbound_enqueue(&east_bound, train); break;
    }
    log_sim(train->train_num, "is ready to go", track_dir);
    res_ties[train->load_time - 1]++;
    pthread_cond_broadcast(&queue_cond);
    pthread_mutex_unlock(&queue_mtx);

    /* Await track opening with lock to enforce single-tracking. */
    pthread_mutex_lock(&track_mtx);
    while (train->track_Xer == false) {
       pthread_cond_wait(&track_cond, &track_mtx);
    }
    log_sim(train->train_num, "is ON the main track going", track_dir);
    usleep(train->Xing_time * US_PER_DEC_SEC); /* "Xing" train. */
    log_sim(train->train_num, "is OFF the main track after going", track_dir);
    /* Update Xing info. */
    train->track_Xed = true;
    train->track_Xer = false;
    pthread_cond_broadcast(&track_cond);
    pthread_mutex_unlock(&track_mtx);

    return NULL;
}

/* Controls track behavior & timing. */
void *track_routine (void *train_void) {
    int trains = *(int *)train_void; /* Convert void type. */

    char last_bound = '\0'; /* Previous dir. */
    char starvation = '\0'; /* Starving dir. */
    train_t *Xing_train = NULL;
    while (trains-- > 0) { 
        /* Dequeue train with lock to avoid concurrent modification. */
        pthread_mutex_lock(&queue_mtx);
        while (!WEST_bound && !EAST_bound && !west_bound && !east_bound) {
            pthread_cond_wait(&queue_cond, &queue_mtx); /* Await ready train. */
        }
        /*
         * Swap dir if starving trains await.
         * Else, dispatch priority direction.
         * Swap dir if opposing trains await.
         * Else, dispatch first loaded train.
         * Else, (west = best) dispatch west.
         */
        if (WEST_bound && starvation != 'E' && (!EAST_bound || last_bound != 'W')) {
            if (last_bound == 'W' && (EAST_bound || east_bound)) 
                starvation = 'E'; /* Last 2 went W, E starving. */
            Xing_train = WEST_bound->train;
            last_bound = 'W';
            inbound_dequeue(&WEST_bound);
        }
        else if (EAST_bound && starvation != 'W' && (!WEST_bound || last_bound != 'E')) {
            if (last_bound == 'E' && (WEST_bound || west_bound)) 
                starvation = 'W'; /* Last 2 went E, W starving. */
            Xing_train = EAST_bound->train;
            last_bound = 'E';
            inbound_dequeue(&EAST_bound);
        }
        else if (west_bound && starvation != 'E' && (!east_bound || last_bound != 'W')) {
            if (last_bound == 'W' && (EAST_bound || east_bound)) 
                starvation = 'E'; /* Last 2 went W, E starving. */
            Xing_train = west_bound->train;
            last_bound = 'W';
            inbound_dequeue(&west_bound);
        }
        else if (east_bound) {
            if (last_bound == 'E' && (WEST_bound || west_bound)) 
                starvation = 'W'; /* Last 2 went E, W starving. */
            Xing_train = east_bound->train;
            last_bound = 'E';
            inbound_dequeue(&east_bound);
        }
        pthread_mutex_unlock(&queue_mtx);

        /* Close track opening with lock to enforce single-tracking. */
        pthread_mutex_lock(&track_mtx);
        /* Update Xing info. */
        Xing_train->track_Xer = true; 
        pthread_cond_broadcast(&track_cond); 
        /* Await track opening with lock to enforce single-tracking. */
        while (Xing_train->track_Xed == false) {
            pthread_cond_wait(&track_cond, &track_mtx);
        }
        Xing_train = NULL;
        pthread_mutex_unlock(&track_mtx);
        if (last_bound == starvation) starvation = '\0'; /* W/E fed. */
    }

    return NULL;
}

/* Create train threads and join them after crossing. */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input.txt>\n", argv[0]);
        exit(1);
    }

    /* Access input.txt for readings. */
    input = fopen(argv[1], "r");
    if (!input) {
        perror("fopen");
        exit(1);
    } 

    /* Access output.txt & overwrite. */
    output = fopen("output.txt", "w");
    if (!output) {
        perror("fopen");
        exit(1);
    } 
    fclose(output);

    /* Access output.txt for appends. */
    output = fopen("output.txt", "a");
    if (!output) {
        perror("fopen");
        exit(1);
    }

    /* Init queues. */
    WEST_bound = NULL;
    EAST_bound = NULL;
    west_bound = NULL;
    east_bound = NULL;

    int num_trains = 0;
    int max_trains = 100;

    clock_gettime(CLOCK_MONOTONIC, &time_0); /* Sim time 0. */
    train_t *train_list = malloc(max_trains * sizeof(train_t));
    if (!train_list) {
        perror("malloc");
        exit(1);
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);

    char WEwe_field;
    int load_field, Xing_field;
    
    /*
     * List each input.txt train & create thread.
     */
    while (
        fscanf(input, " %c %d %d", &WEwe_field, &load_field, &Xing_field) == 3
    ) {
        /* Resize for more trains. */
        if (num_trains >= max_trains) {
            max_trains *= 2;
            train_t *train_temp = realloc(train_list, max_trains * sizeof(train_t));
            if (!train_temp) {
                perror("realloc");
                exit(1);
            }
            train_list = train_temp;
        }

        train_t *train_curr = &train_list[num_trains];

        train_curr->train_num = num_trains;
        train_curr->order_num = load_ties[load_field - 1]++;
        train_curr->train_dir = WEwe_field;
        train_curr->load_time = load_field;
        train_curr->Xing_time = Xing_field;
        train_curr->track_Xed = false;
        train_curr->track_Xer = false;

        if(pthread_create(&train_curr->thread, &attr, train_routine, train_curr)) {
            perror("pthread_create");
            exit(1);    
        }
        num_trains++;
    }
    fclose(input);

    pthread_t track_thread;
    if(pthread_create(&track_thread, &attr, track_routine, &num_trains)) {
        perror("pthread_create"); 
        exit(1);
    } 
    pthread_join(track_thread, NULL);

    for (int i = 0; i < num_trains; i++) {
        if(pthread_join(train_list[i].thread, NULL)) 
            perror("pthread_join"); 
    }
    free(train_list);
    pthread_attr_destroy(&attr);
    fclose(output);

    printf("This is MTS.\n");
    return 0;
}
