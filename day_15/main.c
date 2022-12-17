#include <errno.h>
#include <iso646.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

#include "array.h"
#include "helpers.h"
#include "vec2i.h"

typedef struct {
    vec2i_t p;
    vec2i_t b;
} sensor_t;

ARRAY(sensor_t, sensor_array)
ARRAY(vec2i_t, vec2i_array)

int usage(const char *name) {
    printf("usage: %s row input\n", name);
    printf("\trow: row number to check\n");
    printf("\tinput: path to input file, '-' to use stdin\n");
    return EXIT_FAILURE;
}

bool in(const vec2i_array *vs, vec2i_t v) {
    for (size_t i = 0; i < vs->len; ++i) {
        if (vec2i_equ(vs->data[i], v)) {
            return true;
        }
    }
    return false;
}

void sort_spans(vec2i_array *array) {
    for (size_t i = 0; i < array->len - 1; ++i) {
        for (size_t j = 0; j < array->len - i - 1; ++j) {
            if (array->data[j].x > array->data[j + 1].x) {
                vec2i_t tmp = array->data[j];
                array->data[j] = array->data[j + 1];
                array->data[j + 1] = tmp;
            }
        }
    }
}

typedef struct {
    vec2i_t left, right;
} pair_t;

pair_t merge_spans(vec2i_array *spans) {
    pair_t unmerged = {vec2i(0, 0), vec2i(0, 0)};
    for (size_t i = 0; i < spans->len - 1; ++i) {
        vec2i_t *left = &spans->data[i], *right = &spans->data[i + 1];
        if ((right->x >= left->x and right->x <= left->y) or (left->y >= right->x and left->y <= right->y) or
            (left->y == right->x - 1)) {
            *right = vec2i(MIN(left->x, right->x), MAX(left->y, right->y));
        } else {
            unmerged.left = *left;
            unmerged.right = *right;
        }
    }
    return unmerged;
}

int main(int argc, char *argv[]) {
    if (argc - 2 != 1) {
        return usage(argv[0]);
    }

    int row = 0;
    if (sscanf(argv[1], "%d", &row) != 1) {
        fprintf(stderr, "could not read '%s' as integer\n", argv[1]);
        return EXIT_FAILURE;
    }

    const char *input = argv[argc - 1];
    FILE *fptr = strequ(input, "-") ? stdin : fopen(input, "r");
    if (!fptr) {
        fprintf(stderr, "could not open %s: %s\n", input, strerror(errno));
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t len = 0;

    sensor_array sensors = {0, 0, NULL};
    while (getline(&line, &len, fptr) != -1) {
        if (strequ(line, "\n")) {
            continue;
        }

        sensor_t sensor = {{0, 0}, {0, 0}};
        sscanf(line, "Sensor at x=%d, y=%d: closest beacon is at x=%d, y=%d\n", &sensor.p.x, &sensor.p.y, &sensor.b.x,
               &sensor.b.y);

        sensor_array_append(&sensors, sensor);
    }

    {
        printf("--- Part One ---\n");
        printf("Consult the report from the sensors you just deployed. In the row where y=%d, how many positions "
               "cannot contain a beacon?\n",
               row);

        int min_x = 0, max_x = 0;
        vec2i_array beacons_in_row = {0, 0, NULL};
        for (size_t i = 0; i < sensors.len; ++i) {
            sensor_t s = sensors.data[i];
            int d = abs(s.p.x - s.b.x) + abs(s.p.y - s.b.y) - abs(s.p.y - row);

            if (d < 0) {
                continue;
            }

            if (i == 0) {
                min_x = s.p.x - d;
                max_x = s.p.x + d;
            }

            min_x = s.p.x - d < min_x ? s.p.x - d : min_x;
            max_x = s.p.x + d > max_x ? s.p.x + d : max_x;

            if (s.b.x >= min_x and s.b.x <= max_x and s.b.y == row and not in(&beacons_in_row, s.b)) {
                vec2i_array_append(&beacons_in_row, s.b);
            }
        }

        vec2i_array_free(&beacons_in_row);

        printf("%zu positions cannot contain a beacon in the row y=%d\n",
               (size_t)abs(max_x - min_x) + 1 - beacons_in_row.len, row);
    }

    {
        printf("--- Part Two ---\n");
        printf("Find the only possible position for the distress beacon. What is its tuning frequency?\n");

        uint64_t freq = 0;
        for (int y = 0; y <= 2 * row; ++y) {
            vec2i_array spans = {0, 0, NULL};
            for (size_t i = 0; i < sensors.len; ++i) {
                sensor_t s = sensors.data[i];
                int d = abs(s.p.x - s.b.x) + abs(s.p.y - s.b.y) - abs(s.p.y - y);
                if (d < 0) {
                    continue;
                }

                vec2i_t span = {MAX(0, s.p.x - d), MIN(2 * row, s.p.x + d)};
                vec2i_array_append(&spans, span);
            }
            sort_spans(&spans);
            pair_t unmerged = merge_spans(&spans);
            vec2i_array_free(&spans);
            if (not vec2i_equ(unmerged.left, vec2i(0, 0)) and not vec2i_equ(unmerged.right, vec2i(0, 0))) {
                freq = (uint64_t)(unmerged.left.y + unmerged.right.x) * (uint64_t)row + (uint64_t)y;
                break;
            }
        }

        printf("Its tuning frequency is %lu.\n", freq);
    }

    sensor_array_free(&sensors);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
