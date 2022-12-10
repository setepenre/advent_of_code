#include <errno.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "helpers.h"

int usage(const char *name) {
    printf("usage: %s input\n", name);
    printf("\tinput: path to input file, '-' to use stdin\n");
    return EXIT_FAILURE;
}

typedef struct {
    int start, end;
} pair;

typedef struct {
    pair left, right;
} assignment;

ARRAY(assignment, assignment_array)

static inline bool contains(pair a, pair b) { return b.start >= a.start and b.end <= a.end; }
static inline bool overlaps(pair a, pair b) {
    return (b.start >= a.start and b.start <= a.end) or (b.end >= a.start and b.end <= a.end);
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

    assignment_array assignments = {0, 0, NULL};
    while (getline(&line, &len, fptr) != -1) {
        if (strequ(line, "\n")) {
            continue;
        }

        assignment pairs;
        sscanf(line, "%d-%d,%d-%d\n", &pairs.left.start, &pairs.left.end, &pairs.right.start, &pairs.right.end);
        if (not assignment_array_append(&assignments, pairs)) {
            fprintf(stderr, "could not reallocate %ld bytes: %s\n", assignments.len * sizeof(assignment),
                    strerror(errno));
            return EXIT_FAILURE;
        }
    }

    {
        printf("--- Part One ---\n");
        printf("In how many assignment pairs does one range fully contain the other?\n");
        int count = 0;
        for (size_t i = 0; i < assignments.len; ++i) {
            if (contains(assignments.data[i].left, assignments.data[i].right) or
                contains(assignments.data[i].right, assignments.data[i].left)) {
                count++;
            }
        }
        printf("In %d assignments does one range fully contain the other.\n", count);
    }
    {
        printf("--- Part Two ---\n");
        printf("In how many assignment pairs do the ranges overlap?\n");
        int count = 0;
        for (size_t i = 0; i < assignments.len; ++i) {
            if (overlaps(assignments.data[i].left, assignments.data[i].right) or
                overlaps(assignments.data[i].right, assignments.data[i].left)) {
                count++;
            }
        }
        printf("The rangers overlap in %d assignments.\n", count);
    }

    assignment_array_free(&assignments);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
