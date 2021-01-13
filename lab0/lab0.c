// NAME: Zhengtong Liu
// EMAIL: ericliu2023@g.ucla.edu
// ID: 505375562

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>


struct opts {
    bool valid;
    bool segfault;
    bool catch_flag;
    char* input;
    char* output;
};

void sigsegv_handler (int sig) {
    if (sig == SIGSEGV) 
    {
        fprintf(stderr, "a segmentation fault has bee detected.\n");
        exit(4);
    }
}

void read_options(
    int argc,
    char* argv[],
    struct opts* opts
) {
    opts -> valid = false;
    opts -> segfault = false;
    opts -> catch_flag = false;
    opts -> input = NULL;
    opts -> output = NULL;

    int opt = 0;
    static struct option long_options[] = {
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"segfault", no_argument, 0, 's'},
        {"catch", no_argument, 0, 'c'},
        {0, 0, 0, 0}
    };

    int long_index = 0;
    bool flag = true;
    while ((opt = getopt_long(argc, argv, "", 
                    long_options, &long_index)) != -1) {
            switch (opt)
            {
            case 'i':
                opts -> input = optarg;
                break;
            case 'o':
                opts -> output = optarg;
                break;
            case 's':
                opts -> segfault = true;
                break;
            case 'c':
                opts -> catch_flag = true;
                break;
            default:
                flag = false;
            }
        }
    if (optind < argc)
    {
        flag = false;
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s", argv[optind++]);
        putchar ('\n');
    }
    opts -> valid = flag;
}

int
main (int argc, char **argv)
{
    struct opts options;
    read_options(argc, argv, &options);

    if(!options.valid){
        fprintf(stderr, "%s: Incorrect usage\n", argv[0]);
        fprintf(stderr, "usage: ./lab0 [--input=filename --output=filename --segfault --catch]\n");
        exit(1);
    }

    if(options.input) {
        int ifd;
        if ((ifd = open(options.input, O_RDONLY)) < 0)
        {
            fprintf(stderr, "--input option: ");
            fprintf(stderr, "%s: %s\n", options.input, strerror(errno));
            exit(2);
        }

        close(0);
        dup(ifd);
        close(ifd);
    }

    if(options.output) {
        int ofd;
        if((ofd = creat(options.output, 0666)) < 0)
        {
            fprintf(stderr, "--output option: ");
            fprintf(stderr, "%s: %s\n", options.output, strerror(errno));
            exit(3);
        }

        close(1);
        dup(ofd);
        close(ofd);
    }

    if(options.catch_flag) {
        signal(SIGSEGV, sigsegv_handler);
    }

    if(options.segfault) {
        char* ptr = NULL;
        (*ptr) = 0;
    }

    char c;
    while ((read(0, &c, sizeof(char))) > 0)
        write(1, &c, sizeof(char));
    // char buffer;
    // while (read(0, &buffer, 1) > 0)
    // {
    //     if(buffer != 0)
    //         write(1, &buffer, 1);
    //     else
    //         break;
    // }
    close(0);
    close(1);
    
    exit(0);

}