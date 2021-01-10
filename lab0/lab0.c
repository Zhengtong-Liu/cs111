#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

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
        int ifd = open(options.input, O_RDONLY);
        printf("the file is %s, fd is %d\n", options.input, ifd);
        if(ifd >= 0) {
            close(0);
            dup(ifd);
            close(ifd);
        }
    }

    if(options.output) {
        int ofd = creat(options.output, 0666); //create or open
        printf("the file is %s, fd is %d\n", options.input, ofd);
        if (ofd >= 0) {
            close(1);
            dup(ofd);
            close(ofd);
        }
    }

    if(options.catch_flag) {
        printf("signal to detect segfault has been registered");
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


    printf ("the input file is %s \n", options.input);
    printf ("the output file is %s \n", options.output);
}