#include <errno.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "helpers.h"

ARRAY(long, long_array)

int usage(const char *name) {
    printf("usage: %s input\n", name);
    printf("\tinput: path to input file, '-' to use stdin\n");
    return EXIT_FAILURE;
}

long sum(long *array, size_t n) {
    long acc = 0;
    for (size_t i = 0; i < n; ++i) {
        acc += array[i];
    }
    return acc;
}

int main(int argc, char *argv[]) {
    if (argc - 1 != 1) {
        return usage(argv[0]);
    }

    const char *input = argv[1];
    FILE *fptr = strequ(input, "-") ? stdin : fopen(input, "r");
    if (not fptr) {
        fprintf(stderr, "could not open %s: %s\n", input, strerror(errno));
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t len = 0;

    long_array calories = {0, 0, NULL};
    if (not long_array_append(&calories, 0)) {
        fprintf(stderr, "could not allocate %ld bytes: %s\n", calories.cap * sizeof(long), strerror(errno));
        return EXIT_FAILURE;
    }

    while (getline(&line, &len, fptr) != -1) {
        if (strequ(line, "\n")) {
            if (not long_array_append(&calories, 0)) {
                fprintf(stderr, "could not reallocate %ld bytes: %s\n", calories.cap * sizeof(long), strerror(errno));
                return EXIT_FAILURE;
            }
        } else {
            errno = 0;
            long v = strtol(line, NULL, 10);
            if (errno != 0) {
                fprintf(stderr, "could not convert string '%s' to long: %s\n", line, strerror(errno));
                return EXIT_FAILURE;
            }
            calories.data[calories.len - 1] += v;
        }
    }

    long maximums[3] = {0, 0, 0};
    for (size_t i = 0; i < calories.len; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            if (calories.data[i] > maximums[j]) {
                for (size_t k = 2; k > j; --k) {
                    maximums[k] = maximums[k - 1];
                }
                maximums[j] = calories.data[i];
                break;
            }
        }
    }

    printf("--- Part One ---\n");
    printf("Find the Elf carrying the most Calories. How many total Calories is that Elf carrying?\n");

    printf("The Elf carrying the most Calories is carrying %ld Calories\n", maximums[0]);

    printf("--- Part Two ---\n");
    printf(
        "Find the top three Elves carrying the most Calories. How many Calories are those Elves carrying in total?\n");

    printf("The top three Elves carrying the most Calories are carrying %ld Calories in total\n", sum(maximums, 3));

    long_array_free(&calories);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
