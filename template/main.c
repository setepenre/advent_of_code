#include <errno.h>
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>

#include "helpers.h"

int usage(const char *name) {
    printf("usage: %s input\n", name);
    printf("\tinput: path to input file, '-' to use stdin\n");
    return EXIT_FAILURE;
}

int main(int argc, char *argv[]) {
    if (argc - 1 != 1) {
        return usage(argv[0]);
    }

    const char *input = argv[1];
    FILE *fptr = strequ(input, "-") ? stdin : fopen(input, "r");
    if (!fptr) {
        fprintf(stderr, "could not open %s: %s\n", input, strerror(errno));
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, fptr) != -1) {
    }

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
