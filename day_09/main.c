#include <errno.h>
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "helpers.h"
#include "vec2i.h"

typedef struct {
    char direction;
    size_t steps;
} move_t;

ARRAY(move_t, move_t_array)

vec2i_t delta_from_direction(move_t move) {
    vec2i_t delta = {0, 0};
    switch (move.direction) {
    case 'U':
        delta.y = +1;
        break;
    case 'R':
        delta.x = +1;
        break;
    case 'D':
        delta.y = -1;
        break;
    case 'L':
        delta.x = -1;
        break;
    default:
        break;
    }
    return delta;
}

static inline bool touching(vec2i_t head, vec2i_t tail) {
    return abs(head.x - tail.x) < 2 and abs(head.y - tail.y) < 2;
}

int sign(int n) { return n < 0 ? -1 : +1; }

vec2i_t follow(vec2i_t head, vec2i_t tail) {
    if (touching(head, tail)) {
        return tail;
    }

    vec2i_t diff = {head.x - tail.x, head.y - tail.y};
    vec2i_t delta = {diff.x == 0 ? 0 : sign(diff.x), diff.y == 0 ? 0 : sign(diff.y)};

    return vec2i_add(tail, delta);
}

ARRAY(vec2i_t, vec2i_t_array)

bool vec2i_is_in_array(vec2i_t v, const vec2i_t_array *array) {
    for (size_t i = 0; i < array->len; ++i) {
        if (vec2i_equ(v, array->data[i])) {
            return true;
        }
    }
    return false;
}

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
    if (not fptr) {
        fprintf(stderr, "could not open %s: %s\n", input, strerror(errno));
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t len = 0;

    move_t_array moves = {0, 0, NULL};
    while (getline(&line, &len, fptr) != -1) {
        if (strequ(line, "\n")) {
            continue;
        }

        move_t move = {'\0', 0};
        sscanf(line, "%c %zu\n", &move.direction, &move.steps);

        move_t_array_append(&moves, move);
    }

    {
        printf("--- Part One ---\n");
        printf("Simulate your complete hypothetical series of motions. How many positions does the tail of the rope "
               "visit at least once?\n");

        vec2i_t head = {0, 0}, tail = {0, 0};
        vec2i_t_array tail_positions = {0, 0, NULL};

        vec2i_t_array_append(&tail_positions, tail);
        for (size_t i = 0; i < moves.len; ++i) {
            move_t move = moves.data[i];
            vec2i_t delta = delta_from_direction(move);

            for (size_t j = 0; j < move.steps; ++j) {
                head = vec2i_add(head, delta);
                if (not vec2i_is_in_array(tail = follow(head, tail), &tail_positions)) {
                    vec2i_t_array_append(&tail_positions, tail);
                }
            }
        }

        printf("The tail of the rope visited %zu positions at least once.\n", tail_positions.len);

        vec2i_t_array_free(&tail_positions);
    }

    {
        printf("--- Part Two ---\n");
        printf("Simulate your complete series of motions on a larger rope with ten knots. How many positions does the "
               "tail of the rope visit at least once?\n");

        vec2i_t_array knots = {0, 0, NULL};
        for (size_t i = 0; i < 10; ++i) {
            vec2i_t zero = {0, 0};
            vec2i_t_array_append(&knots, zero);
        }

        vec2i_t_array tail_positions = {0, 0, NULL};
        vec2i_t_array_append(&tail_positions, knots.data[9]);
        for (size_t i = 0; i < moves.len; ++i) {
            move_t move = moves.data[i];
            vec2i_t delta = delta_from_direction(move);

            for (size_t j = 0; j < move.steps; ++j) {
                knots.data[0] = vec2i_add(knots.data[0], delta);
                for (size_t k = 1; k < 10; ++k) {
                    knots.data[k] = follow(knots.data[k - 1], knots.data[k]);
                }
                if (not vec2i_is_in_array(knots.data[9], &tail_positions)) {
                    vec2i_t_array_append(&tail_positions, knots.data[9]);
                }
            }
        }

        printf("The tail of the rope visited %zu positions at least once.\n", tail_positions.len);

        vec2i_t_array_free(&tail_positions);
        vec2i_t_array_free(&knots);
    }

    move_t_array_free(&moves);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
