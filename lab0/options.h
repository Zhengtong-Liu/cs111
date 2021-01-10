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