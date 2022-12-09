#include <errno.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "helpers.h"

typedef struct {
    char direction;
    size_t steps;
} move_t;

ARRAY(move_t, move_t_array)

typedef struct {
    int x, y;
} vec2i_t;

static inline bool vec2i_equ(vec2i_t a, vec2i_t b) { return a.x == b.x and a.y == b.y; }

static inline vec2i_t vec2i_add(vec2i_t a, vec2i_t b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

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

    vec2i_t delta = {head.x - tail.x, head.y - tail.y};
    if (abs(delta.x) > abs(delta.y)) {
        tail.y += delta.y;
        tail.x += sign(delta.x) * (abs(delta.x) - 1);
    } else if (abs(delta.y) > abs(delta.x)) {
        tail.x += delta.x;
        tail.y += sign(delta.y) * (abs(delta.y) - 1);
    } else if (abs(delta.x) == abs(delta.y)) {
        tail.x += sign(delta.x) * (abs(delta.x) - 1);
        tail.y += sign(delta.y) * (abs(delta.y) - 1);
    }

    return tail;
}

ARRAY(vec2i_t, vec2i_t_array)

bool vec2i_is_in_array(vec2i_t v, vec2i_t_array array) {
    for (size_t i = 0; i < array.len; ++i) {
        if (vec2i_equ(v, array.data[i])) {
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
                vec2i_t_array_append(&tail_positions, tail = follow(head, tail));
            }
        }

        vec2i_t_array uniques = {0, 0, NULL};
        for (size_t i = 0; i < tail_positions.len; ++i) {
            if (not vec2i_is_in_array(tail_positions.data[i], uniques)) {
                vec2i_t_array_append(&uniques, tail_positions.data[i]);
            }
        }

        printf("The tail of the rope visited %zu positions at least once.\n", uniques.len);

        vec2i_t_array_free(&uniques);
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
                vec2i_t_array_append(&tail_positions, knots.data[9]);
            }
        }

        vec2i_t_array uniques = {0, 0, NULL};
        for (size_t i = 0; i < tail_positions.len; ++i) {
            if (not vec2i_is_in_array(tail_positions.data[i], uniques)) {
                vec2i_t_array_append(&uniques, tail_positions.data[i]);
            }
        }

        printf("The tail of the rope visited %zu positions at least once.\n", uniques.len);

        vec2i_t_array_free(&uniques);
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
