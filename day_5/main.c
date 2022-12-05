#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "helpers.h"
#include "stack.h"

ARRAY(char *, charptr_array)
STACK(char, char_stack)
ARRAY(char_stack, char_stack_array)

int usage(const char *name) {
    printf("usage: %s input\n", name);
    printf("\tinput: path to input file, '-' to use stdin\n");
    return EXIT_FAILURE;
}

typedef struct {
    size_t quantity, from, to;
} move;

ARRAY(move, move_array)

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

    charptr_array rows = {0, 0, NULL};
    while (getline(&line, &len, fptr) != -1) {
        if (strequ(line, "\n")) {
            break;
        }

        const size_t n_stacks = strlen(line) / 4;

        char *row = calloc(n_stacks + 1, sizeof(char));
        for (size_t i = 0; i < n_stacks; ++i) {
            const char *fmt = i != n_stacks ? "[%c] " : "[%c]\n";
            sscanf(line + i * 4, fmt, row + i);
        }

        for (size_t i = 0; i < n_stacks; ++i) {
            row[i] = row[i] != '\0' ? row[i] : ' ';
        }
        row[n_stacks] = '\0';

        if (!charptr_array_append(&rows, row)) {
            fprintf(stderr, "could not reallocate %ld bytes: %s\n", rows.len * sizeof(char *), strerror(errno));
            return EXIT_FAILURE;
        }
    }

    move_array moves = {0, 0, NULL};
    while (getline(&line, &len, fptr) != 1) {
        if (strequ(line, "\n")) {
            continue;
        }

        move m = {0, 0, 0};
        sscanf(line, "move %zu from %zu to %zu\n", &m.quantity, &m.from, &m.to);
        m.from -= 1;
        m.to -= 1;
        if (!move_array_append(&moves, m)) {
            fprintf(stderr, "could not reallocate %ld bytes: %s\n", moves.len * sizeof(move), strerror(errno));
            return EXIT_FAILURE;
        }
    }

    {
        printf("--- Part One ---\n");
        printf("After the rearrangement procedure completes, what crate ends up on top of each stack?\n");
        char_stack_array stacks = {0, 0, NULL};
        for (size_t i = 1; i < rows.len; ++i) {
            for (size_t j = 0; j < strlen(rows.data[i]); ++j) {
                if (rows.data[rows.len - 1 - i][j] == ' ') {
                    continue;
                }
                if (!(stacks.len > j)) {
                    char_stack stack = {0, 0, NULL};
                    char_stack_array_append(&stacks, stack);
                }
                if (!char_stack_push(&stacks.data[j], rows.data[rows.len - 1 - i][j])) {
                    fprintf(stderr, "could not reallocate %ld bytes: %s\n", stacks.data[j].cap * sizeof(char),
                            strerror(errno));
                    return EXIT_FAILURE;
                }
            }
        }

        for (size_t i = 0; i < moves.len; ++i) {
            move m = moves.data[i];
            char_stack *from = &stacks.data[m.from], *to = &stacks.data[m.to];
            for (size_t j = 0; j < m.quantity; ++j) {
                if (from->len == 0) {
                    continue;
                }
                if (!char_stack_push(to, char_stack_pop(from))) {
                    fprintf(stderr, "could not reallocate %ld bytes: %s\n", to->cap * sizeof(char), strerror(errno));
                    return EXIT_FAILURE;
                }
            }
        }

        char *stack_tops = calloc(stacks.len + 1, sizeof(char));
        for (size_t i = 0; i < stacks.len; ++i) {
            stack_tops[i] = char_stack_top(&stacks.data[i]);
        }
        stack_tops[stacks.len] = '\0';

        printf("After the rearrangement procedure completes, the crates '%s' end up on top of each stack\n",
               stack_tops);

        free(stack_tops);

        for (size_t i = 0; i < stacks.len; ++i) {
            char_stack_free(&stacks.data[i]);
        }
        char_stack_array_free(&stacks);
    }

    {
        printf("--- Part Two ---\n");
        printf("After the rearrangement procedure completes, what crate ends up on top of each stack?\n");
        char_stack_array stacks = {0, 0, NULL};
        for (size_t i = 1; i < rows.len; ++i) {
            for (size_t j = 0; j < strlen(rows.data[i]); ++j) {
                if (rows.data[rows.len - 1 - i][j] == ' ') {
                    continue;
                }
                if (!(stacks.len > j)) {
                    char_stack stack = {0, 0, NULL};
                    char_stack_array_append(&stacks, stack);
                }
                if (!char_stack_push(&stacks.data[j], rows.data[rows.len - 1 - i][j])) {
                    fprintf(stderr, "could not reallocate %ld bytes: %s\n", stacks.data[j].cap * sizeof(char),
                            strerror(errno));
                    return EXIT_FAILURE;
                }
            }
        }

        for (size_t i = 0; i < moves.len; ++i) {
            move m = moves.data[i];
            char_stack *from = &stacks.data[m.from], *to = &stacks.data[m.to];
            char *buffer = calloc(m.quantity, sizeof(char));

            for (size_t j = 0; j < m.quantity; ++j) {
                if (from->len == 0) {
                    continue;
                }
                buffer[j] = char_stack_pop(from);
            }

            for (size_t j = 0; j < m.quantity; ++j) {
                if (buffer[m.quantity - 1 - j] == ' ') {
                    continue;
                }
                if (!char_stack_push(to, buffer[m.quantity - 1 - j])) {
                    fprintf(stderr, "could not reallocate %ld bytes: %s\n", to->cap * sizeof(char), strerror(errno));
                    return EXIT_FAILURE;
                }
            }

            free(buffer);
        }

        char *stack_tops = calloc(stacks.len + 1, sizeof(char));
        for (size_t i = 0; i < stacks.len; ++i) {
            stack_tops[i] = stacks.data[i].len > 0 ? char_stack_top(&stacks.data[i]) : ' ';
        }
        stack_tops[stacks.len] = '\0';

        printf("After the rearrangement procedure completes, the crates '%s' end up on top of each stack\n",
               stack_tops);

        free(stack_tops);

        for (size_t i = 0; i < stacks.len; ++i) {
            char_stack_free(&stacks.data[i]);
        }
        char_stack_array_free(&stacks);
    }

    move_array_free(&moves);

    for (size_t i = 0; i < rows.len; ++i) {
        if (rows.data[i]) {
            free(rows.data[i]);
        }
    }
    charptr_array_free(&rows);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
