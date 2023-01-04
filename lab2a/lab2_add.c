// NAME: Amy Sep
// EMAIL: amyseo@g.ucla.edu
// ID: 505328863

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

int yield = 0;
long num_threads = 1;
long num_itr = 1;
long num_operations;
long lock = 0;
char sync_opt;
long long counter = 0;
pthread_mutex_t mutex;
char test_name_buff[15];

static inline unsigned long get_nanosec(struct timespec* spec){
    unsigned long ret = spec->tv_sec;
    ret = ret * 1000000000 + spec->tv_nsec;
    return ret;
}

void get_test_name() {
    const char *word = "add";
    strcpy(test_name_buff, word);
    if(yield) {
        strcat(test_name_buff, "-yield");
    }
    strcat(test_name_buff, "-");
    switch(sync_opt) {
        case 'm':
        case 's':
        case 'c':
            strcat(test_name_buff, &sync_opt);
            break;
        default:
            strcat(test_name_buff, "none");
    }  
}

void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (yield) {
        if (sched_yield() == -1) {
            fprintf(stderr, "%s\n","ERROR: Failed to schedule yield.");
            exit(1);
        }
    }
    *pointer = sum;
}

void add_mutex(long long *pointer, long long value) {
    if (pthread_mutex_lock(&mutex) != 0) {
        fprintf(stderr, "%s\n","ERROR: Failed to lock mutex.");
        exit(1);
    }
    long long sum = *pointer + value;
    if (yield) {
        if (sched_yield() == -1) {
            fprintf(stderr, "%s\n","ERROR: Failed to schedule yield.");
            exit(1);
        }
    }
    *pointer = sum;
    if (pthread_mutex_unlock(&mutex) != 0) {
        fprintf(stderr, "%s\n","ERROR: Failed to unlock mutex.");
        exit(1);
    }
}

void add_spin_lock(long long *pointer, long long value) {
    while (__sync_lock_test_and_set(&lock, 1));
    long long sum = *pointer + value;
    if (yield) {
        if (sched_yield() == -1) {
            fprintf(stderr, "%s\n","ERROR: Failed to schedule yield.");
            exit(1);
        }
    }
    *pointer = sum;
    __sync_lock_release(&lock);
}

void add_atomic(long long *pointer, long long value){
    long long prev, sum;
    do {
        if(yield){
            if (sched_yield() == -1) {
                fprintf(stderr, "%s\n","ERROR: Failed to schedule yield.");
                exit(1);
            }   
        }
        prev = *pointer;
        sum = prev + value;
    } while (__sync_val_compare_and_swap(pointer, prev, sum) != prev);
}

void* thread_worker(void *arg) {
    for (long i = 0; i < num_itr; i++) {
        switch(sync_opt) {
            case 'm':
                add_mutex(&counter, 1);
                add_mutex(&counter, -1);
                break;
            case 's':
                add_spin_lock(&counter, 1);
                add_spin_lock(&counter, -1);
                break;
            case 'c':
                add_atomic(&counter, 1);
                add_atomic(&counter, -1);
                break;
            default:
                add(&counter, 1);
                add(&counter, -1);
                break;
        }
    }
    return arg;
}

int main (int argc, char **argv) {
    int opt;
    unsigned long total_runtime;
    struct timespec begin, end;
    long time_per_operation;

    static struct option opts[] = {{"threads", required_argument, NULL, 't'},
                                   {"iterations,", required_argument, NULL, 'i'},
                                   {"yield", 0, NULL, 'y'},
                                   {"sync", required_argument, NULL, 's'},
                                   {0, 0, 0, 0}};
    // Get argument
    while ((opt = getopt_long(argc, argv, "", opts, NULL)) != -1) {
        switch (opt) {
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'i':
                num_itr = atoi(optarg);
                break;
            case 'y':
                yield = 1;
                break;
            case 's':
                sync_opt = optarg[0];
                break;
            default:
                fprintf(stderr, "%s\n", "No such argument. Allowed arguments are --threads=num, --iterations=num, --yield, and --sync=m/c/s");
                exit(1);
                break;
        }
    }

    if (sync_opt == 'm') {
        if (pthread_mutex_init(&mutex, NULL) != 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to initialize mutex.");
            exit(1);
        }
    }

    // initialize threads & timer
    if (clock_gettime(CLOCK_MONOTONIC, &begin) < 0) {
        fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
        exit(1);
    }
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));

    for (int i = 0; i < num_threads; i++) {
        if(pthread_create(&threads[i], NULL, &thread_worker, NULL) != 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to create threads.");
            exit(1);
        }
    }
    for(int j = 0; j < num_threads; j++) {
        if (pthread_join(threads[j], NULL) != 0){
            fprintf(stderr, "%s\n", "ERROR: Failed to join threads.");
            exit(1);
        }
    }

    // get runtime
    if (clock_gettime(CLOCK_MONOTONIC, &end) < 0) {
        fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
        exit(1);
    }
    total_runtime = get_nanosec(&end) - get_nanosec(&begin);
    num_operations = num_threads * num_itr * 2;
    time_per_operation = total_runtime/num_operations;

    // print out test case
    get_test_name();
    fprintf(stdout, "%s,%ld,%ld,%ld,%ld,%ld,%lld\n", test_name_buff, num_threads, num_itr, num_operations, total_runtime, time_per_operation, counter);
    free(threads);
    exit(0);
}