#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

int usage(const char *name) {
    printf("usage: %s input\n", name);
    printf("\tinput: path to input file, '-' to use stdin\n");
    return EXIT_FAILURE;
}

typedef struct stack {
    size_t cap, len;
    char *data;
} stack;

bool stack_push(stack *s, char c) {
    if (!s->cap || !s->data) {
        s->cap = 1;
        s->data = calloc(s->cap, sizeof(char));
        if (!s->data) {
            return false;
        }
    }

    if (!(s->len + 1 < s->cap)) {
        s->cap *= 2;
        s->data = realloc(s->data, s->cap * sizeof(char));
        if (!s->data) {
            return false;
        }
    }

    s->data[s->len++] = c;
    return true;
}

char stack_top(const stack *s) { return s->len > 0 ? s->data[s->len - 1] : ' '; }

char stack_pop(stack *s) { return s->len > 0 ? s->data[--s->len] : ' '; }

typedef struct move {
    size_t quantity, from, to;
} move;

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

    size_t crate_data_length = 1;
    char **crate_data = calloc(crate_data_length, sizeof(char *));
    if (!crate_data) {
        fprintf(stderr, "could not allocate %lu: %s\n", crate_data_length * sizeof(char *), strerror(errno));
        return EXIT_FAILURE;
    }
    size_t current = 0, stacks_length = 0;
    while (getline(&line, &len, fptr) != -1) {
        if (strequ(line, "\n")) {
            break;
        }

        if (!(current + 1 < crate_data_length)) {
            crate_data_length *= 2;
            crate_data = realloc(crate_data, crate_data_length * sizeof(char *));
            if (!crate_data) {
                fprintf(stderr, "could not reallocate %ld bytes: %s\n", crate_data_length * sizeof(char *),
                        strerror(errno));
                return EXIT_FAILURE;
            }
        }

        if (!stacks_length) {
            stacks_length = strlen(line) / 4;
        }

        char *crates = calloc(stacks_length + 1, sizeof(char));
        for (size_t i = 0; i < stacks_length; ++i) {
            const char *fmt = i != stacks_length ? "[%c] " : "[%c]\n";
            sscanf(line + i * 4, fmt, crates + i);
        }

        for (size_t i = 0; i < stacks_length; ++i) {
            crates[i] = crates[i] != '\0' ? crates[i] : ' ';
        }
        crates[stacks_length] = '\0';

        crate_data[current++] = crates;
    }
    crate_data_length = current;

    size_t moves_length = 1;
    move *moves = calloc(moves_length, sizeof(move));
    if (!moves) {
        fprintf(stderr, "could not allocate %lu bytes: %s\n", moves_length * sizeof(move), strerror(errno));
        return EXIT_FAILURE;
    }
    current = 0;
    while (getline(&line, &len, fptr) != 1) {
        if (strequ(line, "\n")) {
            continue;
        }

        if (!(current + 1 < moves_length)) {
            moves_length *= 2;
            moves = realloc(moves, moves_length * sizeof(move));
            if (!moves) {
                fprintf(stderr, "could not reallocate %ld bytes: %s\n", moves_length * sizeof(move), strerror(errno));
                return EXIT_FAILURE;
            }
        }

        move m = {0, 0, 0};
        sscanf(line, "move %zu from %zu to %zu\n", &m.quantity, &m.from, &m.to);
        m.from -= 1;
        m.to -= 1;
        moves[current++] = m;
    }
    moves_length = current;

    {
        printf("--- Part One ---\n");
        printf("After the rearrangement procedure completes, what crate ends up on top of each stack?\n");
        stack *stacks = calloc(stacks_length, sizeof(stack));
        for (size_t i = 1; i < crate_data_length; ++i) {
            for (size_t j = 0; j < stacks_length; ++j) {
                if (crate_data[crate_data_length - 1 - i][j] == ' ') {
                    continue;
                }
                if (!stack_push(&stacks[j], crate_data[crate_data_length - 1 - i][j])) {
                    fprintf(stderr, "could not push element on stack: %s", strerror(errno));
                    return EXIT_FAILURE;
                }
            }
        }

        for (size_t i = 0; i < moves_length; ++i) {
            move m = moves[i];
            stack *from = &stacks[m.from], *to = &stacks[m.to];
            for (size_t j = 0; j < m.quantity; ++j) {
                if (!stack_push(to, stack_pop(from))) {
                    fprintf(stderr, "could not push element on stack: %s\n", strerror(errno));
                    return EXIT_FAILURE;
                }
            }
        }

        char *stack_tops = calloc(stacks_length + 1, sizeof(char));
        for (size_t i = 0; i < stacks_length; ++i) {
            stack_tops[i] = stack_top(&stacks[i]);
        }
        stack_tops[stacks_length] = '\0';

        printf("After the rearrangement procedure completes, the crates '%s' end up on top of each stack\n",
               stack_tops);

        free(stack_tops);

        for (size_t i = 0; i < stacks_length; ++i) {
            if (stacks[i].data) {
                free(stacks[i].data);
            }
        }
        free(stacks);
    }

    {
        printf("--- Part Two ---\n");
        printf("After the rearrangement procedure completes, what crate ends up on top of each stack?\n");
        stack *stacks = calloc(stacks_length, sizeof(stack));
        for (size_t i = 1; i < crate_data_length; ++i) {
            for (size_t j = 0; j < stacks_length; ++j) {
                if (crate_data[crate_data_length - 1 - i][j] == ' ') {
                    continue;
                }
                if (!stack_push(&stacks[j], crate_data[crate_data_length - 1 - i][j])) {
                    fprintf(stderr, "could not push element on stack: %s", strerror(errno));
                    return EXIT_FAILURE;
                }
            }
        }

        for (size_t i = 0; i < moves_length; ++i) {
            move m = moves[i];
            stack *from = &stacks[m.from], *to = &stacks[m.to];
            char *buffer = calloc(m.quantity, sizeof(char));

            for (size_t j = 0; j < m.quantity; ++j) {
                buffer[j] = stack_pop(from);
            }

            for (size_t j = 0; j < m.quantity; ++j) {
                if (buffer[m.quantity - 1 - j] == ' ') {
                    continue;
                }
                if (!stack_push(to, buffer[m.quantity - 1 - j])) {
                    fprintf(stderr, "could not push element on stack: %s\n", strerror(errno));
                    return EXIT_FAILURE;
                }
            }

            free(buffer);
        }

        char *stack_tops = calloc(stacks_length + 1, sizeof(char));
        for (size_t i = 0; i < stacks_length; ++i) {
            stack_tops[i] = stack_top(&stacks[i]);
        }
        stack_tops[stacks_length] = '\0';

        printf("After the rearrangement procedure completes, the crates '%s' end up on top of each stack\n",
               stack_tops);

        free(stack_tops);

        for (size_t i = 0; i < stacks_length; ++i) {
            if (stacks[i].data) {
                free(stacks[i].data);
            }
        }
        free(stacks);
    }

    free(moves);

    for (size_t i = 0; i < crate_data_length; ++i) {
        if (crate_data[i]) {
            free(crate_data[i]);
        }
    }
    free(crate_data);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
