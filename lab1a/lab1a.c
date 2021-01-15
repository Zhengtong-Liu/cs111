// NAME: Zhengtong Liu
// EMAIL: ericliu2023@g.ucla.edu
// ID: 505375562

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <getopt.h>

struct termios tattr;


int
main(int argc, char **argv)
{

    int opt = 0;
    static struct option long_options[] = {
        {"shell", no_argument, 0, 's'},
        {0, 0, 0, 0}
    };

    int long_index = 0;
    bool shell_flag = false;
    while ((opt = getopt_long(argc, argv, "",
                    long_options, &long_index)) != -1) {
        switch (opt)
        {
        case 's':
            shell_flag = true;
            printf("--shell evoked\n");
            break;
        default:
            fprintf(stderr, "%s: Incorrect usage\n", argv[0]);
            fprintf(stderr, "usage: ./lab1a [--shell]\n");
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

    if (! isatty(0))
    {
        fprintf(stderr, "File descriptor 0 is not for terminal");
        exit(1);
    }

    if (tcgetattr(0, &tattr) != 0)
    {
        fprintf(stderr, "tcgetattr error\n");
        exit(1);
    }

    struct termios temp;
    tcgetattr(0, &temp);
    temp.c_iflag = ISTRIP;
    temp.c_oflag = 0;
    temp.c_lflag = 0;
    
    // if (tcsetattr(0, TCSANOW, &temp) != 0)
    // {
    //     fprintf(stderr, "tcsetattr error\n");
    //     exit(1);
    // }
    
    char c;
    while ((read(0, &c, sizeof(char))) > 0)
        write(1, &c, sizeof(char));
    
    if (shell_flag)
    {
        int pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "fork failed\n");
            exit(1);
        }
        else if (pid == 0)
        {
            fprintf(stdout, "this is child\n");
            // execute a shell
        }
        else
        {
            fprintf(stdout, "this is parent\n");
        }
    }

    // if (tcsetattr(0, TCSANOW, &tattr) != 0)
    // {
    //     fprintf(stderr, "tcsetattr error\n");
    //     exit(1);
    // }

    exit(0);
}