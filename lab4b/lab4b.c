
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <math.h>
#include <mraa.h>
#include <mraa/aio.h>

// #ifdef DUMMY
// #define MRAA_GPIO_IN 0
// typedef int mraa_aio_context;
// typedef int mraa_gpio_context;

// mraa_aio_context mraa_aio_init (int p) {
//     return 0;
// }

// void mraa_deinit () {

// }

// int mraa_aio_read (mraa_aio_context c) {
//     return 650;
// }

// void mraa_aio_close (mraa_aio_context c) {

// }

// mraa_gpio_context mraa_gpio_init (int p) {
//     return 0;
// }

// void mraa_gpio_dir (mraa_gpio_context c, int d) {

// }

// int mraa_gpio_read (mraa_gpio_context c) {
//     return 0;
// }

// void mraa_gpio_close (mraa_gpio_context c) {

// }

// #else
// #include <mraa.h>
// #include <mraa/aio.h>
// #endif

#define B 4275
#define R0 100000.0
int run_flag = 1;
FILE *log_file = NULL;
char scale = 'F';

mraa_gpio_context button;
mraa_aio_context temper;

struct opts {
    int period;
    char scale;
    char* log_path;
};

void read_options (int argc, char* argv[], struct opts* opts);
float convert_temper_reading (int reading);
void print_current_time ();
void do_when_pushed ();
void my_print(bool to_stdout, char* str);

int main (int argc, char **argv)
{
    struct opts options;
    read_options(argc, argv, &options);

    scale = options.scale;

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

    mraa_gpio_dir(button, MRAA_GPIO_IN);
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, do_when_pushed, NULL);

    // struct pollfd pollStdin = {0, POLLIN, 0};

    while (run_flag)
    {
        print_current_time();
    }


    mraa_gpio_close(button);
    mraa_aio_close(temper);
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
            if (opts -> period <= 0)
            {
                fprintf(stderr, "period cannot be less or equal to 0\n");
                exit(1);
            }
            break;
        case 's':
            opts -> scale = optarg[0];
            if (opts -> scale != 'C' && opts -> scale != 'F')
            {
                fprintf(stderr, "scale can only be F/C\n");
                exit(1);
            }
            break;
        case 'l':
            opts -> log_path = optarg;
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
    struct timespec ts;
    struct tm *tm;
    clock_gettime(CLOCK_REALTIME, &ts);
    tm = localtime(&(ts.tv_sec));
    char current_time[256];
    sprintf(current_time, "%2d:%2d:%2d", tm -> tm_hour, tm -> tm_min, tm -> tm_sec);

    int raw_temp = mraa_aio_read(temper);
    float float_temp = convert_temper_reading(raw_temp);
    int temp = float_temp * 10;

    char output[256];
    sprintf(output, "%s %d.%1d", current_time, temp/10, temp%10);

    my_print(true, output);

}

void do_when_pushed () {
    struct timespec ts;
    struct tm *tm;
    clock_gettime(CLOCK_REALTIME, &ts);
    tm = localtime(&(ts.tv_sec));
    char current_time[256];
    sprintf(current_time, "%2d:%2d:%2d", tm -> tm_hour, tm -> tm_min, tm -> tm_sec);
    
    char output[256];
    sprintf(output, "%s SHUTDOWN\n", current_time);
    my_print(true, output);
    run_flag = 0;
}

void my_print(bool to_stdout, char* str)
{
    if (log_file != NULL)
    {
       fprintf(log_file, "%s\n", str);
       fflush(log_file); 
    }

    if (to_stdout)
        fprintf(stdout, "%s\n", str);
}