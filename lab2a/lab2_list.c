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
    int th_num;
    int it_num;
    char* yield_type;
    char sync_type;
};


char* yield = NULL;
char sync_type;
SortedList_t *listhead;
SortedListElement_t *pool;
long iteration;
// for pthread_mutex lock
pthread_mutex_t mutex;
// for spin lock
long lock = 0;

bool read_options (int argc, char* argv[], struct opts* opts);
void *thread_worker (void *arg);
void segfault_handler ();
static inline long get_nanosec_from_timespec (struct timespec* spec)
{
    long ret = spec -> tv_sec;
    ret = ret * 1000000000 + spec -> tv_nsec;
    return ret;
}

int 
main (int argc, char **argv)
{
    signal(SIGSEGV, segfault_handler);
    struct opts options;
    if (! read_options(argc, argv, &options))
    {
        fprintf(stderr, "invalid command-line parameters\n");
        exit(1);
    }

    long thread = options.th_num;
    iteration = options.it_num;
    long threads[thread];
    pthread_t pthreads[thread];
    sync_type = options.sync_type;
    yield = options.yield_type;
    
    if (yield != NULL)
    {
        int len_yield = strlen(yield);
        for (int i = 0; i < len_yield; i++) {
            if (yield[i] == 'i')
                opt_yield |= INSERT_YIELD;
            else if (yield[i] == 'd')
                opt_yield |= DELETE_YIELD;
            else if (yield[i] == 'l')
                opt_yield |= LOOKUP_YIELD;
        }
    }
    // printf("options are thread: %ld, iterations: %ld, sync type: %c, yield type: %s\n", thread, iteration, sync_type, yield);

    // initialize empty list
    listhead = (SortedList_t *) malloc(sizeof(SortedList_t));
    listhead -> prev = listhead;
    listhead -> next = listhead;
    listhead -> key = NULL;

    pool = (SortedListElement_t *) malloc(thread * iteration * sizeof(SortedListElement_t));
    // srand before calling the rand function
    srand((unsigned int) time(NULL));
    for (int k = 0; k < thread * iteration; k++)
    {
        char *key = (char *) malloc(10 * sizeof(char));
        for (int j = 0; j < 9; j++) {
            // generate a random char from '0' to 'z'
            key[j] = rand() % 75 + 48;
        }
        key[9] = '\0';
        pool[k].key = key;
    }
    if (sync_type == 'm') pthread_mutex_init(&mutex, NULL);

    struct timespec begin, end;
    long diff = 0;

    long i;
    if (clock_gettime(CLOCK_MONOTONIC, &begin) < 0)
    {
        fprintf(stderr, "error when get time (begin): %s\n", strerror(errno));
        exit(1);
    }

    for (i = 0; i < thread; i++) {
        if ((threads[i] = pthread_create(&pthreads[i], NULL, thread_worker, &i)) < 0)
        {
            fprintf(stderr, "error when trying to create new thread\n");
            exit(1);
        }  
    }
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

    diff = get_nanosec_from_timespec(&end) - get_nanosec_from_timespec(&begin);
    long operations = thread * iteration * 3;
    long average_time = diff / operations;

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
    fprintf(stdout, ",%ld,%ld,1,%ld,%ld,%ld\n", thread, iteration, operations, diff, average_time);

    for (int k = 0; k < thread*iteration; k++)
        free((void *)pool[k].key);

    free(pool);
    free(listhead);
    if (sync_type == 'm')
    {
        pthread_mutex_destroy(&mutex);
    }
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
        {0, 0, 0, 0}
    };
    

    int long_index = 0;
    // default settings
    opts -> th_num = 1;
    opts -> it_num = 1;
    opts -> yield_type = NULL;
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
        default:
            fprintf(stderr, "%s: Incorrect usage\n", argv[0]);
            fprintf(stderr, "usage: ./lab2a [--threads=th_num --iterations=it_num --yield]\n");
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

    if (opts -> th_num <= 0 || opts -> it_num <= 0)
        return false;
    
    return true;
}

void *thread_worker(void *arg)
{
 
    long thread_num = *((long*)arg);
    long start_index = thread_num * iteration;
    for (long i = start_index; i < start_index + iteration; i++)
    {
        if (sync_type == 'm')
        {
            pthread_mutex_lock(&mutex);
            SortedList_insert(listhead, &pool[i]);
            pthread_mutex_unlock(&mutex);
        }
        else if (sync_type == 's')
        {
            while (__sync_lock_test_and_set(&lock, 1));
            SortedList_insert(listhead, &pool[i]);
            __sync_lock_release(&lock);
        }
        else
            SortedList_insert(listhead, &pool[i]);
    }
    long len = 0;
    if (sync_type == 'm')
    {
        pthread_mutex_lock(&mutex);
        len = SortedList_length(listhead);
        if (len < 0)
        {
            fprintf(stderr, "the sorted list is corrupted\n");
            exit(2);
        }
        pthread_mutex_unlock(&mutex);
    }
    else if (sync_type == 's')
    {
        while (__sync_lock_test_and_set(&lock, 1));
        len = SortedList_length(listhead);
        if (len < 0)
        {
            fprintf(stderr, "the sorted list is corrupted\n");
            exit(2);
        }
        __sync_lock_release(&lock);
    }
    else
    {
        len = SortedList_length(listhead);
        if (len < 0)
        {
            fprintf(stderr, "the sorted list is corrupted\n");
            exit(2);
        }
    }

    SortedListElement_t *element;
    for (long i = start_index; i < start_index + iteration; i++) {
        if (sync_type == 'm')
        {
            pthread_mutex_lock(&mutex);
            element = SortedList_lookup(listhead, pool[i].key);
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
            pthread_mutex_unlock(&mutex);
        }
        else if (sync_type == 's')
        {
            while (__sync_lock_test_and_set(&lock, 1));
            element = SortedList_lookup(listhead, pool[i].key);
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
            __sync_lock_release(&lock);
        }
        else
        {
            element = SortedList_lookup(listhead, pool[i].key);
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

void segfault_handler ()
{
    fprintf(stderr, "An segmentation fault has been invoked.\n");
    exit(2);
}