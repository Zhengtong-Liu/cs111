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
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/time.h>

#ifdef DUMMY
#define MRAA_GPIO_IN 0
#define MRAA_GPIO_EDGE_RISING 0
typedef int mraa_aio_context;
typedef int mraa_gpio_context;

mraa_aio_context mraa_aio_init (int p) {
    return 0;
}
void mraa_deinit () {

}
int mraa_aio_read (mraa_aio_context c) {
    return 650;
}
void mraa_aio_close (mraa_aio_context c) {

}
mraa_gpio_context mraa_gpio_init (int p) {
    return 0;
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
FILE *log_file = 0;
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
mraa_aio_context temper;

// options structure
struct opts {
    int period;
    char scale;
    char* log_path;
    char* id;
    char* host;
};

int port = -1;
char* id = NULL;
char* host = NULL;

int sockfd = -1;

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
// client connect
int client_connect(char* host_name, unsigned int port);
// process input from server
void process_input_from_server(char* input);

int main (int argc, char **argv)
{
    // get options from command line
    struct opts options;
    read_options(argc, argv, &options);

    scale = options.scale;
    period = options.period;

    id = options.id;
    host = options.host;

    sockfd = client_connect(host, port);

    char id_output[36];
    sprintf(id_output, "ID=%s", id);
    my_print(true, id_output);

    temper = mraa_aio_init(1);

    #ifndef DUMMY
    if (temper == NULL) {
        fprintf(stderr, "Failed to initialize AIO 1\n");
        mraa_deinit();
        return EXIT_FAILURE;
    }
    #endif
    
    // close gpio and aio at exit
    atexit(close_and_exit);

    // poll to get input from stdin
    struct pollfd pollStdin = {sockfd, POLLIN, 0};
    // malloc memory for input buffer
    char* buffer = (char*) malloc(sizeof(char) * 1024);
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
        if (ret < 0)
        {
            fprintf(stderr, "error when polling: %s\n", strerror(errno));
            exit(2);
        }
        if (ret > 0)
            process_input_from_server(buffer);
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
        {"id", required_argument, 0, 'i'},
        {"host", required_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int long_index = 0;
    // default settings
    opts -> period = 1;
    opts -> scale = 'F';
    opts -> id = NULL;
    opts -> host = NULL;

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
        case 'i':
            opts -> id = optarg;
            break;
        case 'h':
            opts -> host = optarg;
            break;
        default:
            fprintf(stderr, "%s: Incorrect usage\n", argv[0]);
            fprintf(stderr, "usage: ./lab4b [--period=N --scale=F/C --log=log_file]\n");
            exit(1);
        }
    }

    if (optind < argc)
    {
        port = atoi(argv[optind]);
        if (port <= 0)
        {
            fprintf(stderr, "invalid port number\n");
            exit(1);
        }
    }
    
    if (log_file == 0)
    {
        fprintf(stderr, "--log option is mandatory\n");
        exit(1);
    }

    if (opts -> host == NULL || strlen(opts -> host) == 0)
    {
        fprintf(stderr, "--host option is mandatory\n");
        exit(1);
    }

    if (opts -> id == NULL || strlen(opts -> id) != 9)
    {
        fprintf(stderr, "--id option is mandatory and id must be 9 digit number\n");
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

    // print to the server if specified
    if (to_stdout)
        dprintf(sockfd, "%s\n", str);
}

void close_and_exit ()
{
    mraa_aio_close(temper);
}

void process_commands (char* buffer)
{
    // first process the input
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

int client_connect (char* host_name, unsigned int port)
{
    struct sockaddr_in serv_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "error with creating socket: %s\n", strerror(errno));
        exit(2);
    }
    struct hostent *server = gethostbyname(host_name);
    if (server == NULL)
    {
        fprintf(stderr, "no such host\n");
        exit(1);
    }
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    int condition = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (condition < 0)
    {
        fprintf(stderr, "fail to connect to the socket on the client side: %s\n", strerror(errno));
        exit(2);
    }
    return sockfd;
}

void process_input_from_server(char* input)
{
    int index = 0;
    char command_buffer[256];
    int ret = read(sockfd, input, 256);
    if (ret < 0)
    {
        fprintf(stderr, "error reading from the server: %s\n", strerror(errno));
        exit(2);
    }
    int k;
    for (k = 0; k < ret && index < 256; k++)
    {
        if (input[k] == '\n')
        {
            process_commands((char *)& command_buffer);
            memset(command_buffer, 0, 256);
            index = 0;
        }
        else
        {
            command_buffer[index] = input[k];
            index++;
        }
    }
}