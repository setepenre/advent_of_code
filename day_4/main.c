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

typedef struct pair {
    int start, end;
} pair;

typedef struct assignment {
    pair left, right;
} assignment;

static inline bool contains(pair a, pair b) { return b.start >= a.start && b.end <= a.end; }
static inline bool overlaps(pair a, pair b) {
    return (b.start >= a.start && b.start <= a.end) || (b.end >= a.start && b.end <= a.end);
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

    size_t assignments_length = 1;
    assignment *assignments = calloc(assignments_length, sizeof(assignment));
    if (!assignments) {
        fprintf(stderr, "could not allocate %ld bytes: %s\n", assignments_length * sizeof(assignment), strerror(errno));
        return EXIT_FAILURE;
    }
    size_t current = 0;
    while (getline(&line, &len, fptr) != -1) {
        if (strequ(line, "\n")) {
            continue;
        }

        if (!(current + 1 < assignments_length)) {
            assignments_length *= 2;
            assignments = realloc(assignments, assignments_length * sizeof(assignment));
            if (!assignments) {
                fprintf(stderr, "could not reallocate %ld bytes: %s\n", assignments_length * sizeof(assignment),
                        strerror(errno));
                return EXIT_FAILURE;
            }
        }

        assignment pairs;
        sscanf(line, "%d-%d,%d-%d\n", &pairs.left.start, &pairs.left.end, &pairs.right.start, &pairs.right.end);
        assignments[current++] = pairs;
    }
    assignments_length = current;

    {
        printf("--- Part One ---\n");
        printf("In how many assignment pairs does one range fully contain the other?\n");
        int count = 0;
        for (size_t i = 0; i < assignments_length; ++i) {
            if (contains(assignments[i].left, assignments[i].right) ||
                contains(assignments[i].right, assignments[i].left)) {
                count++;
            }
        }
        printf("In %d assignments does one range fully contain the other.\n", count);
    }
    {
        printf("--- Part Two ---\n");
        printf("In how many assignment pairs do the ranges overlap?\n");
        int count = 0;
        for (size_t i = 0; i < assignments_length; ++i) {
            if (overlaps(assignments[i].left, assignments[i].right) ||
                overlaps(assignments[i].right, assignments[i].left)) {
                count++;
            }
        }
        printf("The rangers overlap in %d assignments.\n", count);
    }

    free(assignments);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
