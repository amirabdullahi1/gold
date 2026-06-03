#include <time.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#define NS_PER_SEC 1000000000
#define US_PER_DEC_SEC 100000

int rdy_trains;
pthread_mutex_t track_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ready_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t track_cond = PTHREAD_COND_INITIALIZER; 
pthread_cond_t ready_cond = PTHREAD_COND_INITIALIZER; 
struct timespec time_0, time_n;

typedef struct {
    int train_num, load_time, Xing_time;
    char char_dir;
    bool track_Xer, track_Xed; 
    pthread_t thread;
} train_t;

typedef struct bound_node {
    train_t *train;           
    struct bound_node *next;
} bound_node_t;

bound_node_t *west_bound;
bound_node_t *east_bound;

int min (int a, int b) {
    if (a < b) return a; return b;
}

int train_comp (const void *a, const void *b) {
    if (!b) return -1;
    if (!a) return 1;

    const train_t *t1 = (const train_t *)a;
    const train_t *t2 = (const train_t *)b;

    if (t1->load_time < t2->load_time) return -1;
    if (t1->load_time > t2->load_time) return 1;

    if (toupper(t1->char_dir) == toupper(t2->char_dir)) {
        if (t1->char_dir < t2->char_dir) return -1;
        if (t1->char_dir > t2->char_dir) return 1;
    }

    if (t1->train_num < t2->train_num) return -1;
    if (t1->train_num > t2->train_num) return 1;

    return 0;
}

void log_sim (int num, char *ctx, char *dir) {
    clock_gettime(CLOCK_MONOTONIC, &time_n);
    
    double diff_raw_sec = time_n.tv_sec - time_0.tv_sec;
    double diff_raw_nsec = time_n.tv_nsec - time_0.tv_nsec;

    if (diff_raw_nsec < 0) {
        diff_raw_sec -= 1;
        diff_raw_nsec += 1000000000.0;
    }

    double time_diff = diff_raw_sec + diff_raw_nsec / NS_PER_SEC;
    int diff_h = ((int) time_diff / 3600) % 24;
    int diff_m = ((int) time_diff / 60) % 60;
    int diff_s = ((int) time_diff) % 60;
    int diff_ds = (int) fmod(time_diff * 10, 10);

    FILE *output = fopen("output.txt", "a");
    if (!output) {
        perror("fopen Error");
        exit(1);
    }
    fprintf(output, "%02d:%02d:%02d.%1d Train %2d %s %4s\n", diff_h, diff_m, diff_s, diff_ds, num, ctx, dir);
    fclose(output);
}

void inbound_dequeue (bound_node_t **inbound) {
    if (!*inbound) {
        return;
    }

    bound_node_t *curr = *inbound;      
    *inbound = curr->next;              
    curr->next = NULL;
    free(curr);
}

void outbound_enqueue (bound_node_t **outbound, train_t *train) {
    bound_node_t *next = malloc(sizeof(bound_node_t));
    if (!next) {
        perror("malloc Error");
        exit(1);
    } 

    next->train = train;
    next->next = NULL;

    if (!*outbound) {
        *outbound = next;
        return;
    }

    bound_node_t *curr = *outbound;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = next;
}

void *train_routine (void *train_void) {
    train_t *train = (train_t *)train_void;

    usleep(train->load_time * US_PER_DEC_SEC);
    char *str_dir = (toupper(train->char_dir) == 'W') ? "West" : "East";

    pthread_mutex_lock(&ready_mtx);
    log_sim(train->train_num, "is ready to go", str_dir);
    rdy_trains++;
    pthread_cond_signal(&ready_cond);
    pthread_mutex_unlock(&ready_mtx);

    pthread_mutex_lock(&track_mtx);
    while (train->track_Xer == false) {
       pthread_cond_wait(&track_cond, &track_mtx);
    }

    log_sim(train->train_num, "is ON the main track going", str_dir);
    usleep(train->Xing_time * US_PER_DEC_SEC);    
    log_sim(train->train_num, "is OFF the main track after going", str_dir);
    train->track_Xed = true;
    train->track_Xer = false;
    pthread_cond_signal(&track_cond);
    pthread_mutex_unlock(&track_mtx);

    return NULL;
}

void *track_routine (void *train_void) {
    int trains = *(int *)train_void;

    char last_bound = '\0';
    char next_bound = '\0';
    train_t *Xing_train = NULL;
    while (trains--) {
        pthread_mutex_lock(&ready_mtx);
        while (!rdy_trains) {
            pthread_cond_wait(&ready_cond, &ready_mtx);
        }
        rdy_trains--;
        pthread_mutex_unlock(&ready_mtx);

        train_t *west_train = (west_bound) ? west_bound->train : NULL;
        train_t *east_train = (east_bound) ? east_bound->train : NULL;

        bool west_first = (west_train && east_train && west_train->load_time < east_train->load_time);
        bool east_first = (east_train && west_train && east_train->load_time < west_train->load_time);

        if (west_train && (!east_train || west_train->char_dir == 'W' || west_first || (!east_first && last_bound != 'W'))
        ) {
            if (last_bound == 'W') {
                next_bound = 'E';
            }               
            Xing_train = west_train;
            inbound_dequeue(&west_bound);
            last_bound = 'W';
            printf("BBC\n");
        }
        
        else if (
            east_train && next_bound != 'W' && (!west_train || east_first || (!west_first && last_bound != 'E'))
        ) {
            if (last_bound == 'E') {
                next_bound = 'W';
            }               
            Xing_train = east_train;
            inbound_dequeue(&east_bound);
            last_bound = 'E';
            printf("BBB\n");
        }
        else {
            printf("L Bound %c\n", last_bound); 
            printf("N Bound %c\n", next_bound);
            printf("E First %d\n", east_first);
            printf("W loads %d\n", west_train->train_num);
            printf("E loads %d\n", east_train->load_time);
            printf("No Blocks Match\n"); 
        }

        pthread_mutex_lock(&track_mtx);
        Xing_train->track_Xer = true; 
        pthread_cond_broadcast(&track_cond); 
        while(Xing_train->track_Xed == false) {
            pthread_cond_wait(&track_cond, &track_mtx);
        }
        Xing_train = NULL;
        pthread_mutex_unlock(&track_mtx);
        if (last_bound == next_bound) next_bound = '\0';
    }

    return NULL;
}

int main () {
    FILE *output = fopen("output.txt", "w");
    if (!output) {
        perror("fopen Error");
        exit(1);
    } 
    fclose(output);

    FILE *input = fopen("input.txt", "r");
    if (!input) {
        perror("fopen Error");
        exit(1);
    } 

    west_bound = NULL;
    east_bound = NULL;

    int num_trains = 0;
    int max_trains = 100;

    train_t *train_list = malloc(max_trains * sizeof(train_t));
    if (!train_list) {
        perror("malloc Error");
        exit(1);
    }

    clock_gettime(CLOCK_MONOTONIC, &time_0);

    char field1;
    int field2, field3;
    
    while (
        fscanf(input, " %c %d %d", &field1, &field2, &field3) == 3
    ) {
        if (num_trains >= max_trains) {
            max_trains *= 2;
            train_t *train_temp = realloc(train_list, max_trains * sizeof(train_t));
            if (!train_temp) {
                perror("realloc Error");
                exit(1);
            }
            train_list = train_temp;
        }

        train_t *train_curr = &train_list[num_trains];

        train_curr->train_num = num_trains;
        train_curr->char_dir = field1;
        train_curr->load_time = field2;
        train_curr->Xing_time = field3;
        train_curr->track_Xed = false;
        train_curr->track_Xer = false;
        num_trains++;
    }
    fclose(input);

    qsort(train_list, num_trains, sizeof(train_t), train_comp);

    for (int i = 0; i < num_trains; i++) {
        train_t *train_curr = &train_list[i];
        switch (toupper(train_curr->char_dir)) {
            case 'W': outbound_enqueue(&west_bound, train_curr); break;
            case 'E': outbound_enqueue(&east_bound, train_curr); break;
        }
    }

    for (int i = 0; i < num_trains; i++) {
        pthread_create(&train_list[i].thread, NULL, train_routine, &train_list[i]);
    }

    rdy_trains = 0;
    pthread_t track_thread;
    pthread_create(&track_thread, NULL, track_routine, &num_trains);
    pthread_join(track_thread, NULL);

    for (int i = 0; i < num_trains; i++) {
        pthread_join(train_list[i].thread, NULL); 
    }
    free(train_list);

    printf("This is MTS.\n");
    exit(0);
}
