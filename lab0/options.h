// NAME: Zhengtong Liu
// EMAIL: ericliu2023@g.ucla.edu
// ID: 505375562

#include <stdbool.h>

struct opts {
    bool valid;
    bool segfault;
    bool catch_flag;
    char* input;
    char* output;
};

void read_options(
    int argc,
    char* argv[],
    struct opts* opts
);