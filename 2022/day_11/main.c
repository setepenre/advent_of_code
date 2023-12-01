#include <errno.h>
#include <iso646.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "dequeue.h"
#include "helpers.h"

ARRAY(size_t, size_t_array)
DEQUEUE(size_t, size_t_queue)

typedef size_t (*operation_f)(size_t, size_t);
typedef struct {
    size_t x;
    operation_f f;
} operation_t;
size_t operation_call(const operation_t *operation, size_t worry) { return operation->f(operation->x, worry); }

size_t add(size_t value, size_t worry) { return worry + value; }
size_t mul(size_t value, size_t worry) { return worry * value; }
size_t squ(size_t value, size_t worry) {
    (void)value;
    return worry * worry;
}

typedef struct monkey_t monkey_t;
struct monkey_t {
    size_t_queue items;
    operation_t operation;
    size_t divisor;
    size_t siblings[2];
};

ARRAY(monkey_t, monkey_array)

int usage(const char *name) {
    printf("usage: %s input\n", name);
    printf("\tinput: path to input file\n");
    printf("reading file from stdin is not supported.\n");
    return EXIT_FAILURE;
}

int main(int argc, char *argv[]) {
    if (argc - 1 != 1) {
        return usage(argv[0]);
    }

    const char *input = argv[1];
    FILE *fptr = fopen(input, "r");
    if (!fptr) {
        fprintf(stderr, "could not open %s: %s\n", input, strerror(errno));
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t len = 0;

    {
        monkey_array monkeys = {0, 0, NULL};
        while (getline(&line, &len, fptr) != -1) {
            if (strequ(line, "\n")) {
                continue;
            }

            size_t index;
            if (not(sscanf(line, "Monkey %zu:\n", &index) == 1)) {
                continue;
            }

            getline(&line, &len, fptr);
            size_t line_length = strlen(line), control = 0;
            fseek(fptr, -(long)strlen(line), SEEK_CUR);

            size_t_queue items = {0, 0, NULL, NULL};
            getdelim(&line, &len, ':', fptr);
            control += strlen(line);
            while (control < line_length) {
                getdelim(&line, &len, ',', fptr);
                control += strlen(line);

                size_t item;
                sscanf(line, " %zu", &item);
                size_t_queue_push_back(&items, item);
            }
            fseek(fptr, (long)line_length - (long)control, SEEK_CUR);

            getline(&line, &len, fptr);
            operation_t operation = {0, NULL};
            if (strequ(line, "  Operation: new = old * old\n")) {
                operation.f = squ;
            } else if (sscanf(line, "  Operation: new = old + %zu\n", &operation.x) == 1) {
                operation.f = add;
            } else if (sscanf(line, "  Operation: new = old * %zu\n", &operation.x) == 1) {
                operation.f = mul;
            }

            monkey_t monkey = {items, operation, 1, {0}};
            getline(&line, &len, fptr);
            sscanf(line, "  Test: divisible by %zu\n", &monkey.divisor);

            getline(&line, &len, fptr);
            sscanf(line, "    If true: throw to monkey %zu\n", &monkey.siblings[0]);
            getline(&line, &len, fptr);
            sscanf(line, "    If false: throw to monkey %zu\n", &monkey.siblings[1]);

            monkey_array_append(&monkeys, monkey);
        }

        printf("--- Part One ---\n");
        printf("Figure out which monkeys to chase by counting how many items they inspect over 20 rounds. What is the "
               "level of monkey business after 20 rounds of stuff-slinging simian shenanigans?\n");

        size_t_array inspections = {0, 0, NULL};
        for (size_t i = 0; i < monkeys.len; ++i) {
            size_t_array_append(&inspections, 0);
        }

        for (size_t i = 0; i < 20; ++i) {
            for (size_t j = 0; j < monkeys.len; ++j) {
                while (monkeys.data[j].items.len > 0) {
                    size_t worry =
                        operation_call(&monkeys.data[j].operation, size_t_queue_pop_front(&monkeys.data[j].items)) / 3;
                    size_t index = worry % monkeys.data[j].divisor == 0 ? 0 : 1;
                    size_t_queue_push_back(&monkeys.data[monkeys.data[j].siblings[index]].items, worry);
                    inspections.data[j]++;
                }
            }
        }

        size_t most_active[2] = {0};
        for (size_t i = 0; i < inspections.len; ++i) {
            if (inspections.data[i] > most_active[0]) {
                most_active[1] = most_active[0];
                most_active[0] = inspections.data[i];
                continue;
            }
            if (inspections.data[i] > most_active[1]) {
                most_active[1] = inspections.data[i];
                continue;
            }
        }

        printf("The level of monkey business after 20 rounds of stuff-slinging simian shenanigans is %lu.\n",
               most_active[0] * most_active[1]);

        size_t_array_free(&inspections);

        for (size_t i = 0; i < monkeys.len; ++i) {
            size_t_queue_free(&monkeys.data[i].items);
        }
        monkey_array_free(&monkeys);
    }

    rewind(fptr);

    {
        monkey_array monkeys = {0, 0, NULL};
        while (getline(&line, &len, fptr) != -1) {
            if (strequ(line, "\n")) {
                continue;
            }

            size_t index;
            if (not(sscanf(line, "Monkey %zu:\n", &index) == 1)) {
                continue;
            }

            getline(&line, &len, fptr);
            size_t line_length = strlen(line), control = 0;
            fseek(fptr, -(long)strlen(line), SEEK_CUR);

            size_t_queue items = {0, 0, NULL, NULL};
            getdelim(&line, &len, ':', fptr);
            control += strlen(line);
            while (control < line_length) {
                getdelim(&line, &len, ',', fptr);
                control += strlen(line);

                size_t item;
                sscanf(line, " %zu", &item);
                size_t_queue_push_back(&items, item);
            }
            fseek(fptr, (long)line_length - (long)control, SEEK_CUR);

            getline(&line, &len, fptr);
            operation_t operation = {0, NULL};
            if (strequ(line, "  Operation: new = old * old\n")) {
                operation.f = squ;
            } else if (sscanf(line, "  Operation: new = old + %zu\n", &operation.x) == 1) {
                operation.f = add;
            } else if (sscanf(line, "  Operation: new = old * %zu\n", &operation.x) == 1) {
                operation.f = mul;
            }

            monkey_t monkey = {items, operation, 1, {0}};
            getline(&line, &len, fptr);
            sscanf(line, "  Test: divisible by %zu\n", &monkey.divisor);

            getline(&line, &len, fptr);
            sscanf(line, "    If true: throw to monkey %zu\n", &monkey.siblings[0]);
            getline(&line, &len, fptr);
            sscanf(line, "    If false: throw to monkey %zu\n", &monkey.siblings[1]);

            monkey_array_append(&monkeys, monkey);
        }

        printf("--- Part Two ---\n");
        printf("Worry levels are no longer divided by three after each item is inspected; you'll need to find another "
               "way to keep your worry levels manageable. Starting again from the initial state in your puzzle input, "
               "what is the level of monkey business after 10000 rounds?\n");

        size_t_array inspections = {0, 0, NULL};
        for (size_t i = 0; i < monkeys.len; ++i) {
            size_t_array_append(&inspections, 0);
        }

        size_t common_multiple = 1;
        for (size_t i = 0; i < monkeys.len; ++i) {
            common_multiple *= monkeys.data[i].divisor;
        }

        const size_t n_rounds = 10000;
        for (size_t i = 0; i < n_rounds; ++i) {
            for (size_t j = 0; j < monkeys.len; ++j) {
                while (monkeys.data[j].items.len > 0) {
                    size_t worry =
                        operation_call(&monkeys.data[j].operation, size_t_queue_pop_front(&monkeys.data[j].items)) %
                        common_multiple;
                    size_t index = worry % monkeys.data[j].divisor == 0 ? 0 : 1;
                    size_t_queue_push_back(&monkeys.data[monkeys.data[j].siblings[index]].items, worry);
                    inspections.data[j]++;
                }
            }
        }

        size_t most_active[2] = {0};
        for (size_t i = 0; i < inspections.len; ++i) {
            if (inspections.data[i] > most_active[0]) {
                most_active[1] = most_active[0];
                most_active[0] = inspections.data[i];
                continue;
            }
            if (inspections.data[i] > most_active[1]) {
                most_active[1] = inspections.data[i];
                continue;
            }
        }
        printf("\n");

        printf("The level of monkey business after 10000 rounds is %lu.\n", most_active[0] * most_active[1]);

        size_t_array_free(&inspections);

        for (size_t i = 0; i < monkeys.len; ++i) {
            size_t_queue_free(&monkeys.data[i].items);
        }
        monkey_array_free(&monkeys);
    }

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
