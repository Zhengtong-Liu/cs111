#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <getopt.h>

#include "./options.h"

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

    if (argc == 1){
        opts -> valid = false;
        return;
    }
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