// NAME: Zhengtong Liu
// EMAIL: ericliu2023@g.ucla.edu
// ID: 505375562

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include "SortedList.h"

struct opts {
    long th_num;
    long it_num;
    char* yield_type;
    char sync_type;
    int list_num;
};

// yield type
char* yield = NULL;
// sync type
char sync_type;
// sorted list head and list elements pool
SortedList_t *listheads;
SortedListElement_t *pool;
long iteration;
// for pthread_mutex lock
pthread_mutex_t *mutex_locks;
// for spin lock
int *spin_locks;
// for mutex wait time
long wait_time = 0;
// list number
int list_number = 1;

/* read and parse command line options */
bool read_options (int argc, char* argv[], struct opts* opts);
/* thread worker function */
void *thread_worker (void *arg);
/* segmentation fault signal handler */
void segfault_handler (int sig);
/* hash key function*/
static inline int hash_key(const char* key)
{
    int value = (int) key[0];
    return (value % list_number);
}
/* get time difference */
static inline long get_nanosec_from_timespec (struct timespec* spec)
{
    long ret = spec -> tv_sec;
    ret = ret * 1000000000 + spec -> tv_nsec;
    return ret;
}

int 
main (int argc, char **argv)
{
    // register a signal and get parameters from command line input
    signal(SIGSEGV, segfault_handler);
    struct opts options;
    if (! read_options(argc, argv, &options))
    {
        fprintf(stderr, "invalid command-line parameters\n");
        exit(1);
    }

    // thread is # of threads, list_number is # of sub-lists, iteration is # of iterations
    long thread = options.th_num;
    list_number = options.list_num;
    iteration = options.it_num;
    // help to store the value pass to thread worker function
    long threads[thread];
    // need thread number of pthreads
    pthread_t pthreads[thread];
    // sync_type is none, m, or s; yield is none, or combinations of i, d, l
    sync_type = options.sync_type;
    yield = options.yield_type;


    if (sync_type == 'm')
    {
        mutex_locks = (pthread_mutex_t *) malloc (list_number * sizeof(pthread_mutex_t));
        if (mutex_locks == NULL)
        {
            fprintf(stderr, "cannot allocate memory for mutex locks\n");
            exit(1);
        }
    }
    else if (sync_type == 's')
    {
        spin_locks = malloc (list_number * sizeof(int));
        if (spin_locks == NULL)
        {
            fprintf(stderr, "cannot allocate memory for spin locks\n");
            exit(1);
        }
        for (int k = 0; k < list_number; k++)
            spin_locks[k] = 0;
    }

    
    // if yield option has parameters, interpret the parameters
    if (yield != NULL)
    {
        int len_yield = strlen(yield);
        for (int i = 0; i < len_yield; i++) {
            if (yield[i] == 'i')
                opt_yield = opt_yield | INSERT_YIELD;
            else if (yield[i] == 'd')
                opt_yield = opt_yield | DELETE_YIELD;
            else if (yield[i] == 'l')
                opt_yield = opt_yield | LOOKUP_YIELD;
        }
    }

    // initialize empty list
    listheads = (SortedList_t *) malloc(list_number * sizeof(SortedList_t));
    if (listheads == NULL)
    {
        fprintf(stderr, "cannot allocate memory for list heads\n");
        exit(1);
    } 

    for (int k = 0; k < list_number; k++)
    {
        listheads[k].prev = &listheads[k];
        listheads[k].next = &listheads[k];
        listheads[k].key = NULL;
    }

    // initialize sorted list elements pool
    pool = (SortedListElement_t *) malloc(thread * iteration * sizeof(SortedListElement_t));
    if (pool == NULL)
    {
        fprintf(stderr, "cannot allocate memory for pool (storing sorted list elements)\n");
        exit(1);
    }





    // srand before calling the rand function
    srand((unsigned int) time(NULL));
    // assign random keys to sorted list elements
    for (long k = 0; k < thread * iteration; k++)
    {
        char *key = (char *) malloc(10 * sizeof(char));
        if (key == NULL)
        {
            fprintf(stderr, "cannot allocate memory for key (sorted list element key)\n");
            exit(1);
        }
        // generate a random char from '0' to 'z'
        for (int j = 0; j < 9; j++)
            key[j] = rand() % 75 + 48;
        key[9] = '\0';
        pool[k].key = key;
    }
    // initialize mutex lock if needed
       if (sync_type == 'm') {
        for (int k = 0; k < list_number; k++)
        {
            if (pthread_mutex_init(mutex_locks + k, NULL) != 0) {
                fprintf(stderr, "error when initializing the mutex lock\n");
                exit(1);
            }
        }
    }

    // start recording time
    struct timespec begin, end;
    long diff = 0;

    long i;
    if (clock_gettime(CLOCK_MONOTONIC, &begin) < 0)
    {
        fprintf(stderr, "error when get time (begin): %s\n", strerror(errno));
        exit(1);
    }

    // create threads
    for (i = 0; i < thread; i++) {
        threads[i] = i;
        if (pthread_create(&pthreads[i], NULL, thread_worker, &threads[i]) < 0)
        {
            fprintf(stderr, "error when trying to create new thread\n");
            exit(1);
        }
    }
    // join threads
    for (i = 0; i < thread; i++) {
        if (pthread_join(pthreads[i], NULL) < 0)
        {
            fprintf(stderr, "error when trying to join a terminated thread\n");
            exit(1);
        }
    }

    if (clock_gettime(CLOCK_MONOTONIC, &end) < 0)
    {
        fprintf(stderr, "error when get time (end): %s\n", strerror(errno));
        exit(1);
    }

    // calculate time difference
    diff = get_nanosec_from_timespec(&end) - get_nanosec_from_timespec(&begin);
    long operations = thread * iteration * 3;
    long average_time = diff / operations;

    // calculate average mutex lock wait time
    long average_wait_mutex = wait_time / thread;


    // check the length of the list
    for (int k = 0; k < list_number; k++)
    {
        if (SortedList_length(listheads + k) != 0)
        {
            fprintf(stderr, "error: the length of the list is not zero\n");
            exit(2);
        }
    }

    // print outputs to csv
    fprintf(stdout, "list-");
    switch (opt_yield)
    {
    case 0:
        fprintf(stdout, "none-");
        break;
    case 1:
        fprintf(stdout, "i-");
        break;
    case 2:
        fprintf(stdout, "d-");
        break;
    case 3:
        fprintf(stdout, "id-");
        break;
    case 4:
        fprintf(stdout, "l-");
        break;
    case 5:
        fprintf(stdout, "il-");
        break;
    case 6:
        fprintf(stdout, "dl-");
        break;
    case 7:
        fprintf(stdout, "idl-");
    default:
        break;
    }   
    switch (sync_type)
    {
    case 'm':
        fprintf(stdout, "m");
        break;
    case 's':
        fprintf(stdout, "s");
        break;
    default:
        fprintf(stdout, "none");
        break;
    }
    fprintf(stdout, ",%ld,%ld,%d,%ld,%ld,%ld,%ld\n", thread, iteration, list_number, operations, diff, average_time, average_wait_mutex);

    for (long k = 0; k < thread*iteration; k++)
        free((void *)pool[k].key);

    // free memory, finish mutex lock if needed and exit without error
    free(pool);
    free(listheads);

    if (sync_type == 'm')
    {
        for (int k = 0; k < list_number; k++)
            pthread_mutex_destroy(mutex_locks + k);
        free(mutex_locks);
    }
    else if (sync_type == 's')
        free(spin_locks);
    exit(0);
}

bool read_options (int argc, char* argv[], struct opts* opts)
{
    int opt = 0;
    static struct option long_options[] = {
        {"threads", required_argument, 0, 't'},
        {"iterations", required_argument, 0, 'i'},
        {"yield", required_argument, 0, 'y'},
        {"sync", required_argument, 0, 's'},
        {"lists", required_argument, 0, 'l'},
        {0, 0, 0, 0}
    };
    

    int long_index = 0;
    // default settings
    opts -> th_num = 1;
    opts -> it_num = 1;
    opts -> yield_type = NULL;
    opts -> list_num = 1;
    // for sync_type
    char c;
    // for yield type
    char * opt_yield;
    int arg_len, k;
    while ((opt = getopt_long(argc, argv, "",
                    long_options, &long_index)) != -1) {
        switch (opt)
        {
        case 't':
            opts -> th_num = atoi(optarg);
            break;
        case 'i':
            opts -> it_num = atoi(optarg);
            break;
        case 'y':
            opt_yield = optarg;
            arg_len = strlen(opt_yield);
            if (arg_len <= 0)
            {
                fprintf(stderr, "please provide an valid parameter for --yield=[idl]\n");
                exit(1);
            }
            for (k = 0; k < arg_len; k++)
            {
                if (opt_yield[k] != 'i' && opt_yield[k] != 'd' && opt_yield[k] != 'l')
                {
                    fprintf(stderr, "unrecognized arg for --yield=[idl]: %s\n", optarg);
                    exit(1);
                }
            }
            opts -> yield_type = optarg;
            break;
        case 's':
            c = optarg[0];
            if (c != 'm' && c != 's')
            {
                fprintf(stderr, "unrecognized sync type for --sync=[ms]: %c\n", c);
                exit(1);
            }
            opts -> sync_type = optarg[0];
            break;
        case 'l':
            opts -> list_num = atoi(optarg);
            break;
        default:
            fprintf(stderr, "%s: Incorrect usage\n", argv[0]);
            fprintf(stderr, "usage: ./lab2a [--threads=th_num --iterations=it_num --yield=[idl] --sync=[ms]]\n");
            exit(1);
        }
    }

    if (optind < argc)
    {
        fprintf (stderr, "non-option ARGV-elements: ");
        while (optind < argc)
            fprintf(stderr, "%s", argv[optind++]);
        fprintf (stderr, "\n");
        exit(1);
    }

    if (opts -> th_num <= 0 || opts -> it_num <= 0 || opts -> list_num <= 0)
        return false;
    
    return true;
}

void *thread_worker(void *arg)
{
    struct timespec start_time, end_time;

    long thread_num = *((long*)arg);
    long start_index = thread_num * iteration;
    // insert elements with or without lock
    for (long i = start_index; i < start_index + iteration; i++)
    {
        int list_num = hash_key(pool[i].key);
        if (sync_type == 'm')
        {
            clock_gettime(CLOCK_MONOTONIC, &start_time);
            pthread_mutex_lock(mutex_locks + list_num);
            clock_gettime(CLOCK_MONOTONIC, &end_time);

            SortedList_insert(listheads + list_num, pool + i);
            wait_time += get_nanosec_from_timespec(&end_time) - get_nanosec_from_timespec(&start_time);

            pthread_mutex_unlock(mutex_locks + list_num);
        }
        else if (sync_type == 's')
        {
            clock_gettime(CLOCK_MONOTONIC, &start_time);
            while (__sync_lock_test_and_set(spin_locks + list_num, 1));
            clock_gettime(CLOCK_MONOTONIC, &end_time);

            SortedList_insert(listheads + list_num, pool + i);
            wait_time += get_nanosec_from_timespec(&end_time) - get_nanosec_from_timespec(&start_time);

            __sync_lock_release(spin_locks + list_num);

        }
        else if (sync_type != 'm' && sync_type != 's')
            SortedList_insert(listheads + list_num, pool + i);
    }
    // get sorted list length with and without lock
    long len = 0;
    if (sync_type == 'm')
    {
        for (int k = 0; k < list_number; k++)
        {
            clock_gettime(CLOCK_MONOTONIC, &start_time);
            pthread_mutex_lock(mutex_locks + k);
            clock_gettime(CLOCK_MONOTONIC, &end_time);

            long sub_len = SortedList_length(listheads + k);
            if (sub_len < 0)
            {
                fprintf(stderr, "sub-list %d is corrupted\n", k);
                exit(2);
            }
            len += sub_len;

            wait_time += get_nanosec_from_timespec(&end_time) - get_nanosec_from_timespec(&start_time);
            pthread_mutex_unlock(mutex_locks + k);
        }
   
    }
    else if (sync_type == 's')
    {
        for (int k = 0; k < list_number; k++)
        {
            clock_gettime(CLOCK_MONOTONIC, &start_time);
            while (__sync_lock_test_and_set(spin_locks + k, 1));
            clock_gettime(CLOCK_MONOTONIC, &end_time);
            long sub_len = SortedList_length(listheads + k);
            if (sub_len < 0)
            {
                fprintf(stderr, "sub-list %d is corrupted\n", k);
                exit(2);
            }
            len += sub_len;

            wait_time += get_nanosec_from_timespec(&end_time) - get_nanosec_from_timespec(&start_time);
            __sync_lock_release(spin_locks + k);
        }
    }
    else if (sync_type != 'm' && sync_type != 's')
    {
        for (int k = 0; k < list_number; k++)
        {
            long sub_len = SortedList_length(listheads + k);
            if (sub_len < 0)
            {
                fprintf(stderr, "sub-list %d is corrupted\n", k);
                exit(2);
            }
            len += sub_len;
        }
    }

    // look up and delete sorted list elements with and without lock
    SortedListElement_t *element;
    for (long i = start_index; i < start_index + iteration; i++) {

        int list_num = hash_key(pool[i].key);
        if (sync_type == 'm')
        {
            clock_gettime(CLOCK_MONOTONIC, &start_time);
            pthread_mutex_lock(mutex_locks + list_num);
            clock_gettime(CLOCK_MONOTONIC, &end_time);

            element = SortedList_lookup(listheads + list_num, pool[i].key);
            if (element == NULL)
            {
                fprintf(stderr, "element not found in this sorted list\n");
                exit(2);
            }
            if (SortedList_delete(element) == 1)
            {
                fprintf(stderr, "prev/next of this element is corrupted\n");
                exit(2);
            }
            wait_time += get_nanosec_from_timespec(&end_time) - get_nanosec_from_timespec(&start_time);

            pthread_mutex_unlock(mutex_locks + list_num);

        }
        else if (sync_type == 's')
        {
            clock_gettime(CLOCK_MONOTONIC, &start_time);
            while (__sync_lock_test_and_set(spin_locks + list_num, 1));
            clock_gettime(CLOCK_MONOTONIC, &end_time);

            element = SortedList_lookup(listheads + list_num, pool[i].key);
            if (element == NULL)
            {
                fprintf(stderr, "element not found in this sorted list\n");
                exit(2);
            }
            if (SortedList_delete(element) == 1)
            {
                fprintf(stderr, "prev/next of this element is corrupted\n");
                exit(2);
            }
            wait_time += get_nanosec_from_timespec(&end_time) - get_nanosec_from_timespec(&start_time);

            __sync_lock_release(spin_locks + list_num);

        }
        else if (sync_type != 'm' && sync_type != 's')
        {
            element = SortedList_lookup(listheads + list_num, pool[i].key);
            if (element == NULL)
            {
                fprintf(stderr, "element not found in this sorted list\n");
                exit(2);
            }
            if (SortedList_delete(element) == 1)
            {
                fprintf(stderr, "prev/next of this element is corrupted\n");
                exit(2);
            }
        }
    }
    
    return arg;
}

void segfault_handler (int sig)
{
    if (sig == SIGSEGV)
    {
        fprintf(stderr, "An segmentation fault has been invoked.\n");
        exit(2);
    }
}