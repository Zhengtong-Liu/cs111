// NAME: Zhengtong Liu
// EMAIL: ericliu2023@g.ucla.edu
// ID: 505375562

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

struct termios tattr;
struct opts {
    char* port;
    char* file_name;
    bool compress_flag;
    bool valid;
};

void restore_and_exit (int exit_status)
{
    if (tcsetattr(0, TCSANOW, &tattr) != 0)
    {
        fprintf(stderr, "tcsetattr error(restore time): %s\n", strerror(errno));
        exit(1);
    }
    exit(exit_status);
}


void read_options (int argc, char* argv[], struct opts* opts)
{
    int opt = 0;
    static struct option long_options[] = {
        {"port", required_argument, 0, 'p'},
        {"log", required_argument, 0, 'l'},
        {"compress", no_argument, 0, 'c'},
        {0, 0, 0, 0}
    };

    int long_index = 0;
    opts -> port = NULL;
    opts -> file_name = NULL;
    opts -> compress_flag = false;
    while ((opt = getopt_long(argc, argv, "",
                    long_options, &long_index)) != -1) {
        switch (opt)
        {
        case 'p':
            opts -> port = optarg;
            break;
        case 'l':
            opts -> file_name = optarg;
            break;
        case 'c':
            opts -> compress_flag = true;
            break;
        default:
            fprintf(stderr, "%s: Incorrect usage\n", argv[0]);
            fprintf(stderr, "usage: ./lab1b-client [--port=PORT --log=FILENAME --compress]\n");
            restore_and_exit(1);
        }
    }

    if (optind < argc)
    {
        fprintf (stderr, "non-option ARGV-elements: ");
        while (optind < argc)
            fprintf(stderr, "%s", argv[optind++]);
        fprintf (stderr, "\n");
        restore_and_exit(1);
    }

    return;
}

void terminal_setup ()
{
    if (! isatty(0))
    {
        fprintf(stderr, "File descriptor 0 is not for terminal");
        exit(1);
    }

    if (tcgetattr(0, &tattr) != 0)
    {
        fprintf(stderr, "tcgetattr(1st time) error: %s\n", strerror(errno));
        exit(1);
    }

    struct termios temp;
    if (tcgetattr(0, &temp) != 0)
    {
        fprintf(stderr, "tcgetattr(2nd time) error: %s\n", strerror(errno));
        exit(1);
    }

    temp.c_iflag = ISTRIP; temp.c_oflag = 0; temp.c_lflag = 0;
    if (tcsetattr(0, TCSANOW, &temp) != 0)
    {
        fprintf(stderr, "tcsetattr error(set time): %s\n", strerror(errno));
        exit(1);
    }
}


int
main (int argc, char **argv)
{
    terminal_setup();

    struct opts options;
    read_options(argc, argv, &options);

    // --port= mandatory
    if (!options.port)
    {
        fprintf(stderr, "--port=PORT option is mandatory\n");
        restore_and_exit(1);
    }
    else
    {
        printf("port: %s\n", options.port);
    }
    
    if (options.file_name)
    {
        printf("file_name: %s\n", options.file_name);
    }

    if (options.compress_flag)
    {
        printf("compress_flag is set\n");
    }

    restore_and_exit(0);
}