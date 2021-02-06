#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>

struct opts {
    int th_num;
    int it_num;
    bool opt_yield;
    char sync_type;
};

long long sum = 0;
bool yield = false;
char sync_type;

// for pthread_mutex lock
pthread_mutex_t mutex;
// for spin lock
long lock = 0;

bool read_options (int argc, char* argv[], struct opts* opts);
void add(long long *pointer, long long value);
void mutex_lock_add(long long *pointer, long long value);
void spin_lock_add(long long *pointer, long long value);
void atomic_add(long long *pointer, long long value);
void *thread_worker (void *arg);
static inline long get_nanosec_from_timespec (struct timespec* spec)
{
    long ret = spec -> tv_sec;
    ret = ret * 1000000000 + spec -> tv_nsec;
    return ret;
}

int 
main (int argc, char **argv)
{
    struct opts options;
    if (! read_options(argc, argv, &options))
    {
        fprintf(stderr, "invalid command-line parameters\n");
        exit(1);
    }

    long thread = options.th_num;
    long iteration = options.it_num;
    long threads[thread];
    pthread_t pthreads[thread];
    sync_type = options.sync_type;

    if (sync_type == 'm') pthread_mutex_init(&mutex, NULL);

    struct timespec begin, end;
    long diff = 0;

    int i;
    if (clock_gettime(CLOCK_MONOTONIC, &begin) < 0)
    {
        fprintf(stderr, "error when get time (begin): %s\n", strerror(errno));
        exit(1);
    }

    for (i = 0; i < options.th_num; i++) {
        if ((threads[i] = pthread_create(&pthreads[i], NULL, thread_worker, &iteration)) < 0)
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
    long operations = thread * iteration * 2;
    long average_time = diff / operations;

    
    fprintf(stdout, "add-none, %ld, %ld, %ld, %ld, %ld, %lld\n", thread, iteration, operations, diff, average_time, sum);
    fprintf(stdout, "add-yield-none, %ld, %ld, %ld, %ld, %ld, %lld\n", thread, iteration, operations, diff, average_time, sum);
}

bool read_options (int argc, char* argv[], struct opts* opts)
{
    int opt = 0;
    static struct option long_options[] = {
        {"threads", required_argument, 0, 't'},
        {"iterations", required_argument, 0, 'i'},
        {"yield", no_argument, 0, 'y'},
        {"sync", required_argument, 0, 's'},
        {0, 0, 0, 0}
    };
    

    int long_index = 0;
    // default settings
    opts -> th_num = 1;
    opts -> it_num = 1;
    opts -> opt_yield = false;
    // for sync_type
    char c;

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
            opts -> opt_yield = true;
            break;
        case 's':
            c = optarg[0];
            if (c != 'm' && c != 's' && c != 'c')
            {
                fprintf(stderr, "Unrecognized sync type for --sync: %c\n", c);
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
    long iter = *((long*)arg), i = 0;
    for (i=0; i<iter; i++)
    {
        switch (sync_type)
        {
        case 'm':
            mutex_lock_add(&sum, 1);
            break;
        case 's':
            spin_lock_add(&sum, 1);
            break;
        case 'c':
            atomic_add(&sum, 1);
            break;
        default:
            add(&sum, 1);
            break;
        }

    } 
    for (i=0; i<iter; i++)
    {
       switch (sync_type)
        {
        case 'm':
            mutex_lock_add(&sum, -1);
            break;
        case 's':
            spin_lock_add(&sum, -1);
            break;
        case 'c':
            atomic_add(&sum, -1);
            break;
        default:
            add(&sum, -1);
            break;
        }  
    } 

    return arg;
}

void add(long long *pointer, long long value) 
{
    long long sum = *pointer + value;
    if (yield)
        sched_yield();
    *pointer = sum;
}

void mutex_lock_add(long long *pointer, long long value)
{
    pthread_mutex_lock(&mutex);
    long long sum = *pointer + value;
    if (yield)
        sched_yield();
    *pointer = sum;
    pthread_mutex_unlock(&mutex);
}

void spin_lock_add(long long *pointer, long long value)
{
    while (__sync_lock_test_and_set(&lock, 1));
    long long sum = *pointer + value;
    if (yield)
        sched_yield();
    *pointer = sum;
    __sync_lock_release(&lock);
}

void atomic_add(long long *pointer, long long value)
{
    long long prev, sum;
    do {
        prev = *pointer;
        if (yield){
            sched_yield();
        }
        sum = prev + value;
    } while (__sync__bool_compare_and_swap(pointer, prev, sum) == false);
}
