#include <errno.h>
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>

#include "helpers.h"

int usage(const char *name) {
    printf("usage: %s input\n", name);
    printf("\tinput: path to input file\n");
    printf("reading file from stdin is not supported.\n");
    return EXIT_FAILURE;
}

bool all_different(const char *b, size_t n) {
    for (size_t i = 0; i < n - 1; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            if (b[i] == b[j]) {
                return false;
            }
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    if (argc - 1 != 1) {
        return usage(argv[0]);
    }

    const char *input = argv[1];
    FILE *fptr = fopen(input, "r");
    if (not fptr) {
        fprintf(stderr, "could not open %s: %s\n", input, strerror(errno));
        return EXIT_FAILURE;
    }

    {
        printf("--- Part One ---\n");
        printf("How many characters need to be processed before the first start-of-packet marker is detected?\n");

        const size_t len = 4;
        size_t count = 0;
        char buffer[len];
        for (char c = (char)fgetc(fptr); c != EOF; c = (char)fgetc(fptr)) {
            ++count;
            memmove(buffer, buffer + 1, len - 1);
            buffer[len - 1] = c;

            if (count >= len and all_different(buffer, len)) {
                break;
            }
        }

        printf("%zu characters need to processed before the first start-of-packet marker is detected.\n", count);
    }

    rewind(fptr);

    {
        printf("--- Part Two ---\n");
        printf("How many characters need to be processed before the first start-of-message marker is detected?\n");

        const size_t len = 14;
        size_t count = 0;
        char buffer[len];
        for (char c = (char)fgetc(fptr); c != EOF; c = (char)fgetc(fptr)) {
            ++count;
            memmove(buffer, buffer + 1, len - 1);
            buffer[len - 1] = c;

            if (count >= len and all_different(buffer, len)) {
                break;
            }
        }

        printf("%zu characters need to processed before the first start-of-message marker is detected.\n", count);
    }

    fclose(fptr);

    return EXIT_SUCCESS;
}
