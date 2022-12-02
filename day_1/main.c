#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

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
    if (!fptr) {
        fprintf(stderr, "could not open %s: %s\n", input, strerror(errno));
        return EXIT_FAILURE;
    }

    printf("--- Part One ---\n");
    printf("Find the Elf carrying the most Calories. How many total Calories is that Elf carrying?\n");

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    size_t calories_length = 1;
    long *calories = calloc(calories_length, sizeof(long));
    if (!calories) {
        fprintf(stderr, "could not allocate %ld bytes: %s\n", calories_length * sizeof(long), strerror(errno));
        return EXIT_FAILURE;
    }

    size_t current = 0;
    calories[current] = 0;
    while ((nread = getline(&line, &len, fptr)) != -1) {
        if (strequ(line, "\n")) {
            if (!(current + 1 < calories_length)) {
                calories_length *= 2;
                calories = realloc(calories, calories_length * sizeof(long));
                if (!calories) {
                    fprintf(stderr, "could not reallocate %ld bytes: %s\n", calories_length * sizeof(long),
                            strerror(errno));
                    return EXIT_FAILURE;
                }
            }
            calories[++current] = 0;
        } else {
            errno = 0;
            long v = strtol(line, NULL, 10);
            if (errno != 0) {
                fprintf(stderr, "could not convert string '%s' to long: %s\n", line, strerror(errno));
            }
            calories[current] += v;
        }
    }
    calories_length = current;

    long maximums[3] = {0, 0, 0};
    for (size_t i = 0; i < calories_length; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            if (calories[i] > maximums[j]) {
                for (size_t k = 2; k > j; --k) {
                    maximums[k] = maximums[k - 1];
                }
                maximums[j] = calories[i];
                break;
            }
        }
    }

    printf("The Elf carrying the most Calories is carrying %ld Calories\n", maximums[0]);

    printf("--- Part Two ---\n");
    printf(
        "Find the top three Elves carrying the most Calories. How many Calories are those Elves carrying in total?\n");

    printf("The top three Elves carrying the most Calories are carrying %ld Calories in total\n", sum(maximums, 3));

    free(calories);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
