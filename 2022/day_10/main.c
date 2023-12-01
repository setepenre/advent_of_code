#include <errno.h>
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "helpers.h"

typedef enum { NOOP = 0, ADDX } op_t;

typedef struct {
    op_t op;
    int arg;
    size_t cycles;
} instruction_t;

ARRAY(instruction_t, instruction_array)

typedef struct {
    size_t cycle;
    int x;
} cpu_t;

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

    instruction_array instructions = {0, 0, NULL};
    while (getline(&line, &len, fptr) != -1) {
        if (strequ(line, "\n")) {
            continue;
        }

        char op[4];
        int arg = 0;
        if (sscanf(line, "%s %d\n", op, &arg) != 2) {
            sscanf(line, "%s\n", op);
        }

        instruction_t instruction = {strequ(op, "addx") ? ADDX : NOOP, arg, strequ(op, "addx") ? 2 : 1};
        instruction_array_append(&instructions, instruction);
    }

    {
        printf("--- Part One ---\n");
        printf("Find the signal strength during the 20th, 60th, 100th, 140th, 180th, and 220th cycles. What is the sum "
               "of these six signal strengths?\n");

        int signal_strength = 0;
        cpu_t cpu = {1, 1};
        for (size_t i = 0; i < instructions.len; ++i) {
            instruction_t instruction = instructions.data[i];
            for (size_t j = 0; j < instruction.cycles; ++j) {
                switch (instruction.op) {
                case NOOP:
                    ++cpu.cycle;
                    break;
                case ADDX:
                    ++cpu.cycle;
                    cpu.x += (j == instruction.cycles - 1) ? instruction.arg : 0;
                    break;
                }
                if (((int)cpu.cycle - 20) % 40 == 0) {
                    signal_strength += (int)cpu.cycle * cpu.x;
                }
            }
        }

        printf("The sum of these six signals strengths is %d.\n", signal_strength);
    }

    {
        printf("--- Part Two ---\n");
        printf("Render the image given by your program. What eight capital letters appear on your CRT?\n");

        cpu_t cpu = {1, 1};
        for (size_t i = 0; i < instructions.len; ++i) {
            instruction_t instruction = instructions.data[i];
            for (size_t j = 0; j < instruction.cycles; ++j) {
                int x = (int)(cpu.cycle - 1) % 40;
                if (x == 0 and i != 0) {
                    printf("\n");
                }
                printf("%c", abs(x - cpu.x) > 1 ? '.' : '#');
                switch (instruction.op) {
                case NOOP:
                    ++cpu.cycle;
                    break;
                case ADDX:
                    ++cpu.cycle;
                    cpu.x += (j == instruction.cycles - 1) ? instruction.arg : 0;
                    break;
                }
            }
        }
        printf("\n");
    }

    instruction_array_free(&instructions);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
