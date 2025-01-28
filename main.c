#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "utils.h"

int main(ARG_UNUSED int argc, ARG_UNUSED char *argv[]) {
    printf("Hello World\n");

    const char *const filename = "immutable.txt";

    int fd = open(filename, O_CREAT, O_WRONLY);
    if (fd == -1) {
        fprintf(stderr, "Failed to open '%s'", filename);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
