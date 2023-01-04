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
#include "SortedList.h"

#define KEY_LENGTH  16
#define ID_YIELD    0x03
#define IL_YIELD    0x05
#define DL_YIELD    0x06
#define IDL_YIELD   0x07

static const char alpha[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
int opt_yield;
long num_threads = 1;
long num_itr = 1;
long num_operations;
long num_elements;
long num_lists = 1;
long *lock;
char sync_opt;
pthread_mutex_t *mutex;
SortedListElement_t *list, *listheads;
char** keys;
char test_name_buff[15];

static inline unsigned long get_nanosec(struct timespec* spec){
    unsigned long ret = spec->tv_sec;
    ret = ret * 1000000000 + spec->tv_nsec;
    return ret;
}

void segfault_handler() {
    fprintf(stderr, "%s\n", "ERROR: Segmentation fault.");
    exit(2);
}

long hash(const char* key) {
    long total = 0;
    for (int i = 0; i < KEY_LENGTH; i++) {
        total += key[i];
    }
    long h = total%num_lists;
    return h;
}

void get_test_name() {
    // list-yieldopts-syncopts
    const char *word = "list";
    strcpy(test_name_buff, word);
    switch(opt_yield) {
        case INSERT_YIELD:
            strcat(test_name_buff, "-i");
            break;
        case DELETE_YIELD:
            strcat(test_name_buff, "-d");
            break;
        case ID_YIELD:
            strcat(test_name_buff, "-id");
            break;
        case LOOKUP_YIELD:
            strcat(test_name_buff, "-l");
            break;
        case IL_YIELD:
            strcat(test_name_buff, "-il");
            break;
        case DL_YIELD:
            strcat(test_name_buff, "-dl");
            break;
        case IDL_YIELD:
            strcat(test_name_buff, "-idl");
            break;
        default:
            strcat(test_name_buff, "-none");
            break;
    }
    strcat(test_name_buff, "-");
    switch(sync_opt) {
        case 'm':
            strcat(test_name_buff, "m");
            break;
        case 's':
            strcat(test_name_buff, "s");
            break;
        default:
            strcat(test_name_buff, "none");
            break;
    }
}

char* get_random_key() {
    char* cur_key = malloc((KEY_LENGTH + 1) * sizeof(char));
    int random_index;
    for(int i = 0; i < KEY_LENGTH; i++) {
        random_index = rand() % (sizeof(alpha) -1);
        cur_key[i] = alpha[random_index];
    }
    cur_key[KEY_LENGTH] = '\0';
    return cur_key;
}

void* thread_worker(void *arg) {
    long startIndex = *((long*)arg);
    long total = startIndex + num_itr;
    long lock_wait_time = 0;
    struct timespec lock_start, lock_end;

    // insert the elements into the list
     switch(sync_opt) {
        case 'm':
            for(long i = startIndex; i < total; i++) {
                long sublist_num = hash(list[i].key);
                if (clock_gettime(CLOCK_MONOTONIC, &lock_start) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
                    exit(1);
                }
                if (pthread_mutex_lock(&mutex[sublist_num]) != 0) {
                    fprintf(stderr, "%s\n","ERROR: Failed to lock mutex.");
                    exit(1);
                }
                if (clock_gettime(CLOCK_MONOTONIC, &lock_end) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
                    exit(1);
                }
                lock_wait_time += (get_nanosec(&lock_end) - get_nanosec(&lock_start));
                SortedList_insert(&listheads[sublist_num], &list[i]);
                if (pthread_mutex_unlock(&mutex[sublist_num]) != 0) {
                    fprintf(stderr, "%s\n","ERROR: Failed to unlock mutex.");
                    exit(1);
                }
            }  
            break;
        case 's':
            for(long i = startIndex; i < total; i++) {
                long sublist_num = hash(list[i].key);
                if (clock_gettime(CLOCK_MONOTONIC, &lock_start) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
                    exit(1);
                }
                while (__sync_lock_test_and_set(&lock[sublist_num], 1));
                if (clock_gettime(CLOCK_MONOTONIC, &lock_end) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
                    exit(1);
                }
                lock_wait_time += (get_nanosec(&lock_end) - get_nanosec(&lock_start));
                SortedList_insert(&listheads[sublist_num], &list[i]);
                __sync_lock_release(&lock[sublist_num]);
            }
            break;
        default:
            for(long i = startIndex; i < total; i++) {
                long sublist_num = hash(list[i].key);
                SortedList_insert(&listheads[sublist_num], &list[i]);
            }
            break;
        }

    // get length of list
    long length = 0;
    switch(sync_opt) {
        case 'm':
            for (long i = startIndex; i < total; i++) {
                long sublist_num = hash(list[i].key);
                if (clock_gettime(CLOCK_MONOTONIC, &lock_start) < 0) {
                        fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
                        exit(1);
                }
                if (pthread_mutex_lock(&mutex[sublist_num]) != 0) {
                    fprintf(stderr, "%s\n","ERROR: Failed to lock mutex.");
                    exit(1);
                }
                if (clock_gettime(CLOCK_MONOTONIC, &lock_end) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
                    exit(1);
                }
                lock_wait_time += (get_nanosec(&lock_end) - get_nanosec(&lock_start));
                length += SortedList_length(&listheads[sublist_num]);
                if (pthread_mutex_unlock(&mutex[sublist_num]) != 0) {
                    fprintf(stderr, "%s\n","ERROR: Failed to unlock mutex.");
                    exit(1);
                }
            }
            break;
        case 's':
            for (long i = startIndex; i < total; i++) {
                long sublist_num = hash(list[i].key);
                if (clock_gettime(CLOCK_MONOTONIC, &lock_start) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
                    exit(1);
                }
                while (__sync_lock_test_and_set(&lock[sublist_num], 1));
                if (clock_gettime(CLOCK_MONOTONIC, &lock_end) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
                    exit(1);
                }
                lock_wait_time += (get_nanosec(&lock_end) - get_nanosec(&lock_start));
                length += SortedList_length(&listheads[sublist_num]);
                __sync_lock_release(&lock[sublist_num]);
            }
            break;
        default:
            for (long i = startIndex; i < total; i++) {
                long sublist_num = hash(list[i].key);
                length += SortedList_length(&listheads[sublist_num]);
            }
            break;
    }

    if (length < 0) {
        fprintf(stderr, "%s\n", "ERROR: List length is less than 0.");
        exit(2);
    }

    // delete the list   
    switch(sync_opt) {
        case 'm':
            for(int i = startIndex; i < total; i++) {
                long sublist_num = hash(list[i].key);
                if (clock_gettime(CLOCK_MONOTONIC, &lock_start) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
                    exit(1);
                }
                if (pthread_mutex_lock(&mutex[sublist_num]) != 0) {
                    fprintf(stderr, "%s\n","ERROR: Failed to lock mutex.");
                    exit(1);
                }
                if (clock_gettime(CLOCK_MONOTONIC, &lock_end) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
                    exit(1);
                }
                lock_wait_time += (get_nanosec(&lock_end) - get_nanosec(&lock_start));
                SortedListElement_t *tmp_m = SortedList_lookup(&listheads[sublist_num], list[i].key);
                if (tmp_m == NULL) {
                    fprintf(stderr, "%s\n", "ERROR: Could not lookup list element.");
                    exit(2);
                }
                else {
                    if (SortedList_delete(tmp_m)) {
                        fprintf(stderr, "%s\n", "ERROR: Failed to delete element in list.");
                        exit(2);
                    }
                }
                if (pthread_mutex_unlock(&mutex[sublist_num]) != 0) {
                    fprintf(stderr, "%s\n","ERROR: Failed to unlock mutex.");
                    exit(1);
                }
            }
            break;
        case 's':
            for(int i = startIndex; i < total; i++) {
                long sublist_num = hash(list[i].key);
                if (clock_gettime(CLOCK_MONOTONIC, &lock_start) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
                    exit(1);
                }
                while(__sync_lock_test_and_set (&lock[sublist_num], 1));
                if (clock_gettime(CLOCK_MONOTONIC, &lock_end) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
                    exit(1);
                }
                lock_wait_time += (get_nanosec(&lock_end) - get_nanosec(&lock_start));
                SortedListElement_t *tmp_s = SortedList_lookup(&listheads[sublist_num], list[i].key);
                if (tmp_s == NULL) {
                    fprintf(stderr, "%s\n", "ERROR: Could not lookup list element.");
                    exit(2);
                }
                else {
                    if (SortedList_delete(tmp_s)) {
                        fprintf(stderr, "%s\n", "ERROR: Failed to delete element in list.");
                        exit(2);
                    }
                }
                __sync_lock_release(&lock[sublist_num]);
            }
            break;
        default: ;
             for(int i = startIndex; i < total; i++) {
                long sublist_num = hash(list[i].key);
                SortedListElement_t *tmp = SortedList_lookup(&listheads[sublist_num], list[i].key);
                if (tmp == NULL) {
                    fprintf(stderr, "%s\n", "ERROR: Could not lookup list element.");
                    exit(2);
                }
                else {
                    if (SortedList_delete(tmp)) {
                        fprintf(stderr, "%s\n", "ERROR: Failed to delete element in list.");
                        exit(2);
                    }
                }
             }
            break;
    }
    return (void*) lock_wait_time;
}

int main (int argc, char **argv) {
    int opt;
    unsigned long total_runtime;
    struct timespec begin, end;
    unsigned long time_per_operation;
    signal(SIGSEGV, segfault_handler);
    long total_lock_wait_time = 0;

    static struct option opts[] = {{"threads", required_argument, NULL, 't'},
                                   {"iterations,", required_argument, NULL, 'i'},
                                   {"yield", required_argument, NULL, 'y'},
                                   {"sync", required_argument, NULL, 's'},
                                   {"lists", required_argument, NULL, 'l'},
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
                if (strlen(optarg) > 3) {
                    fprintf(stderr, "%s\n", "No such argument. Allowed arguments for --yield are 'i', 'd', 'l'.");
                    exit(1);
                }
                for(long unsigned int i = 0; i < strlen(optarg); i++) {
                    switch(optarg[i]){
                        case 'i':
                            opt_yield |= INSERT_YIELD;
                            break;
                        case 'd':
                            opt_yield |= DELETE_YIELD;
                            break;
                        case 'l':
                            opt_yield |= LOOKUP_YIELD;
                            break;
                        default:
                            fprintf(stderr, "%s\n", "No such argument. Allowed arguments for --yield are 'i', 'd', 'l'.");
                            exit(1);
                            break;
                    }
                }
                break;
            case 's':
                sync_opt = optarg[0];
                if (sync_opt != 'm' && sync_opt != 's') {
                    fprintf(stderr, "%s\n", "No such argument. Allowed arguments for --sync are 'm' and 's'.");
                    exit(1);
                }
                break;
            case 'l':
                num_lists = atoi(optarg);
                if (num_lists < 1) {
                    fprintf(stderr, "%s\n", "ERROR: Bad argument. Number of lists must be a positive integer.");
                    exit(1);
                }
                break;
            default:
                fprintf(stderr, "%s\n", "No such argument. Allowed arguments are --threads=num, --iterations=num, --yield=i/d/l");
                exit(1);
                break;
        }
    }

    if(sync_opt == 'm') {
        mutex = malloc(num_lists * sizeof(pthread_mutex_t));
        for(long i = 0; i < num_lists; i++) {
            if (pthread_mutex_init(&mutex[i], NULL) != 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to initialize mutex.");
                exit(1);
            }
        }
    }
    if(sync_opt == 's') {
        lock = malloc(num_lists * sizeof(long));
        for(long i = 0; i < num_lists; i++) {
            lock[i] = 0;
        }
    }

    listheads = malloc(num_lists * sizeof(SortedList_t));
    for(long i = 0; i < num_lists; i++) {
        listheads[i].next = &listheads[i];
        listheads[i].prev = &listheads[i];
        listheads[i].key = NULL;
    }

    num_elements = num_threads * num_itr;   
    list = malloc(num_elements * sizeof(SortedListElement_t));

    // initialize list with random keys
    srand(time(NULL));
    for (long i = 0; i < num_elements; i++) {
        list[i].key = get_random_key();
    }

    // initialize threads & timer
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    if (clock_gettime(CLOCK_MONOTONIC, &begin) < 0){
        fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
        exit(1);
    }

    long starting_points[num_threads];
    for(long i = 0; i < num_threads; i++) {
        starting_points[i] = i * num_itr;
    }

    for (long i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, &thread_worker, (void*)(&starting_points[i])) != 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to create threads.");
            exit(1);
        };
    }

    void **wait_time = (void*) malloc(sizeof(void**));
    for(int j = 0; j < num_threads; j++) {
        if (pthread_join(threads[j], wait_time) != 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to join threads.");
            exit(1);
        }
        // printf("%ld\n", total_lock_wait_time);
        total_lock_wait_time += (long)*wait_time;
    }

    // get runtime
    if (clock_gettime(CLOCK_MONOTONIC, &end) < 0) {
        fprintf(stderr, "%s\n", "ERROR: Failed to get clock time.");
        exit(1);
    }

    total_runtime = get_nanosec(&end) - get_nanosec(&begin);
    num_operations = num_threads * num_itr * 3;
    time_per_operation = total_runtime/num_operations;
    unsigned long wait_per_lock = total_lock_wait_time/num_operations;

    // print out test case
    get_test_name();
    fprintf(stdout, "%s,%ld,%ld,%ld,%ld,%ld,%ld,%ld\n", test_name_buff, num_threads, num_itr, num_lists, num_operations, total_runtime, time_per_operation, wait_per_lock);
    free(threads);
    free(list);
    free(listheads);
    free(wait_time);
    free(mutex);
    free(lock);
    exit(0);
}