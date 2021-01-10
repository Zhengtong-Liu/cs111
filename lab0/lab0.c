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

#include "./options.h"

void sigsegv_handler (int sig) {
    if (sig == SIGSEGV) 
    {
        fprintf(stderr, "a segmentation fault has bee detected.\n");
        exit(4);
    }
}

int
main (int argc, char **argv)
{
    struct opts options;
    read_options(argc, argv, &options);

    if(!options.valid){
        fprintf(stderr, "%s: Incorrect usage\n", argv[0]);
        fprintf(stderr, "usage: [--input=filename --output=filename --segfault --catch]\n");
        exit(1);
    }

    if(options.input) {
        int ifd;
        if ((ifd = open(options.input, O_RDONLY)) < 0)
        {
            fprintf(stderr, "--input option\n");
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
            fprintf(stderr, "--output option\n");
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

    char buffer;
    while (read(0, &buffer, 1) > 0)
    {
        if(buffer != 0)
            write(1, &buffer, 1);
        else
            break;
    }
    exit(0);

}