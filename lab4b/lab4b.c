// NAME: ZHENGTONG LIU
// ID: 505375562
// EMAIL: ericliu2023@g.ucla.edu

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>

#ifdef DUMMY
#define MRAA_GPIO_IN 0
#define MRAA_GPIO_EDGE_RISING 0
typedef char* mraa_aio_context;
typedef char* mraa_gpio_context;

mraa_aio_context mraa_aio_init (int p) {
    char* dummy = "dummy";
    return dummy;
}
void mraa_deinit () {

}
int mraa_aio_read (mraa_aio_context c) {
    return 650;
}
void mraa_aio_close (mraa_aio_context c) {

}
mraa_gpio_context mraa_gpio_init (int p) {
    char* dummy = "dummy";
    return dummy;
}
void mraa_gpio_dir (mraa_gpio_context c, int d) {

}
void mraa_gpio_close (mraa_gpio_context c) {

}

void mraa_gpio_isr (mraa_gpio_context c, int edge, void (* fptr) (void *), void* arg) {

}
#else
#include <mraa.h>
#include <mraa/aio.h>
#endif


#define B 4275
#define R0 100000.0

// whether to run, based on START or STOP (default set to 1)
int run_flag = 1;
// the log file 
FILE *log_file = NULL;
// temperature scale (default set to F)
char scale = 'F';
// period, set to 0, will be set to 1 as default
int period = 0;
// to keep track of the next print time (whether to print)
time_t start_time = 0;

// for getting time
struct timespec ts;
struct tm *tm;

// gpio and aio
mraa_gpio_context button;
mraa_aio_context temper;

// options structure
struct opts {
    int period;
    char scale;
    char* log_path;
};

// read options from command line inputs
void read_options (int argc, char* argv[], struct opts* opts);
// convert temperature from raw reading to F or C
float convert_temper_reading (int reading);
// print the time with the temperature
void print_current_time ();
// do when the button is pushed
void do_when_pushed ();
// close the gpio and aio 
void close_and_exit ();
// print to stdout (or not) and log file
void my_print(bool to_stdout, char* str);
// process commands from stdin
void process_commands (char* buffer);

int main (int argc, char **argv)
{
    // get options from command line
    struct opts options;
    read_options(argc, argv, &options);

    scale = options.scale;
    period = options.period;

    // initialize gpio and aio
    button = mraa_gpio_init(60);
    if (button == NULL) {
        fprintf(stderr, "Failed to initialize GPIO 60\n");
        mraa_deinit();
        return EXIT_FAILURE;
    }
    temper = mraa_aio_init(1);
    if (temper == NULL) {
        fprintf(stderr, "Failed to initialize AIO 1\n");
        mraa_deinit();
        return EXIT_FAILURE;
    }

    // close gpio and aio at exit
    atexit(close_and_exit);

    mraa_gpio_dir(button, MRAA_GPIO_IN);
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, do_when_pushed, NULL);

    // poll to get input from stdin
    struct pollfd pollStdin = {STDIN_FILENO, POLLIN, 0};
    // malloc memory for input buffer
    char* buffer = (char*) malloc(sizeof(char) * 256);
    if (buffer == NULL)
    {
        fprintf(stderr, "cannot allocate enough memory for buffer\n");
        exit(1);
    }

    while (true)
    {
        // print time with temperature
        print_current_time();

        // if receiving input from stdin, process input buffer, fgets would read
        // until newline char, end of file
        int ret = poll(&pollStdin, 1, 0);
        if (ret > 0)
        {
            fgets(buffer, 256, stdin);
            // fprintf(stdout, "input buffer is %s\n", buffer);
            process_commands(buffer);
        }

    }

    if (buffer != NULL) free(buffer);
    exit(0);

}

void read_options (int argc, char* argv[], struct opts* opts)
{
    int opt = 0;
    static struct option long_options[] = {
        {"period", required_argument, 0, 'p'},
        {"scale", required_argument, 0, 's'},
        {"log", required_argument, 0, 'l'},
        {0, 0, 0, 0}
    };
    
    int long_index = 0;
    // default settings
    opts -> period = 1;
    opts -> scale = 'F';

    while ((opt = getopt_long(argc, argv, "",
                    long_options, &long_index)) != -1) {
        switch (opt)
        {
        case 'p':
            opts -> period = atoi(optarg);
            // if period is less than or equal to 0, the period is invalid
            if (opts -> period <= 0)
            {
                fprintf(stderr, "period cannot be less or equal to 0\n");
                exit(1);
            }
            break;
        case 's':
            opts -> scale = optarg[0];
            // if the scale is not C or F, then scale is invalid
            if (opts -> scale != 'C' && opts -> scale != 'F')
            {
                fprintf(stderr, "scale can only be F/C\n");
                exit(1);
            }
            break;
        case 'l':
            opts -> log_path = optarg;
            // if cannot open (create) with write permission, then the log file path is invalid
            log_file = fopen(opts -> log_path, "w+");
            if (log_file == NULL) {
                fprintf(stderr, "cannot open the file: %s: %s\n", opts -> log_path, strerror(errno));
                exit(1);
            }
            break;
        default:
            fprintf(stderr, "%s: Incorrect usage\n", argv[0]);
            fprintf(stderr, "usage: ./lab4b [--period=N --scale=F/C --log=log_file]\n");
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
    
}

float convert_temper_reading (int reading)
{
    float R = 1023.0/((float) reading) - 1.0;
    R = R0 * R;

    float C = 1.0 / (log (R/R0)/B + 1/298.15) - 273.15;
    float F = (C * 9)/5 + 32;

    if (scale == 'C')
        return C;
    else
        return F;
}

void print_current_time ()
{
    clock_gettime(CLOCK_REALTIME, &ts);
    // if run flag is true, and we are in the next period, then print
    if (run_flag && ts.tv_sec >= start_time)
    {
        // get local time
        tm = localtime(&(ts.tv_sec));
        char current_time[256];
        sprintf(current_time, "%.2d:%.2d:%.2d", tm -> tm_hour, tm -> tm_min, tm -> tm_sec);

        // get temperature and convert it to specified scale
        int raw_temp = mraa_aio_read(temper);
        float float_temp = convert_temper_reading(raw_temp);

        char output[256];
        sprintf(output, "%s %.1f", current_time, float_temp);

        // print to stdout and log file (if specified)
        my_print(true, output);
        // set the time of the start of the next period
        start_time = ts.tv_sec + period;
    }

}


void do_when_pushed () {
    // print time and shut down
    clock_gettime(CLOCK_REALTIME, &ts);
    tm = localtime(&(ts.tv_sec));
    char current_time[256];
    sprintf(current_time, "%.2d:%.2d:%.2d", tm -> tm_hour, tm -> tm_min, tm -> tm_sec);
    
    char output[256];
    sprintf(output, "%s SHUTDOWN", current_time);
    my_print(true, output);
    
    exit(0);
}

void my_print(bool to_stdout, char* str)
{
    // print to log file if specified
    if (log_file != NULL)
    {
       fprintf(log_file, "%s\n", str);
       fflush(log_file); 
    }

    // print to stdout if specified
    if (to_stdout)
        fprintf(stdout, "%s\n", str);
}

void close_and_exit ()
{
    mraa_gpio_close(button);
    mraa_aio_close(temper);
}

void process_commands (char* buffer)
{
    // first process the input
    buffer[strlen(buffer) - 1] = '\0';
    while (*buffer == '\t' || *buffer == ' ')
        buffer++;
    
    // switch requests of different commands
    if (strcmp(buffer, "START") == 0) {
        my_print(false, buffer);

        // fprintf(stdout, "the running flag is %d\n", run_flag);

        run_flag = 1;
        // fprintf(stdout, "the running flag is %d\n", run_flag);
    }
    else if (strcmp(buffer, "STOP") == 0) {
        my_print(false, buffer);
        run_flag = 0;
    }
    else if (strcmp(buffer, "SCALE=F") == 0) {
        my_print(false, buffer);
        scale = 'F';
    }
    else if (strcmp(buffer, "SCALE=C") == 0) {
        my_print(false, buffer);
        scale = 'C';
    }
    else if (strcmp(buffer, "OFF") == 0) {
        my_print(false, buffer);
        do_when_pushed();
    }
    else if (strncmp(buffer, "PERIOD=", 7) == 0) {
        char* str_period = buffer;
        str_period = str_period + 7;
        // fprintf(stdout, "period string is %s\n", str_period);
        if (str_period != NULL)
        {
            // while (str_period != NULL)
            // {
            //     if (isdigit(str_period[0]) > 0)
            //         continue;
            //     else
            //         return;
            //     str_period++;
            // }
            int temp = atoi(str_period);
            if (temp <= 0)
                return;
            else
                period = temp;
            // fprintf(stdout, "period is %d\n", period);
        }
        my_print(false, buffer);
    }
    else if (strncmp(buffer, "LOG", 3) == 0) {
        my_print(false, buffer);
    }

}