#include <errno.h>
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "helpers.h"

ARRAY(int, int_array)

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

    int_array calibrations = {1, 0, calloc(1, sizeof(int))}, calibrations_all_digits = {1, 0, calloc(1, sizeof(int))};

    char *digits_strings[] = {"one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
    while (getline(&line, &len, fptr) != -1) {
        char digits[3] = "";
        for (char *c = line; *c != '\0'; ++c) {
            if (*c >= '0' and *c <= '9') {
                if (digits[0] == '\0') {
                    digits[0] = *c;
                } else {
                    digits[1] = *c;
                }
            }
        }

        if (digits[1] == '\0') {
            digits[1] = digits[0];
        }

        errno = 0;
        int_array_append(&calibrations, (int)strtol(digits, NULL, 10));
        if (errno != 0) {
            fprintf(stderr, "could not convert string '%s' to long: %s\n", digits, strerror(errno));
            return EXIT_FAILURE;
        }

        char *line_interpreted = calloc(len, sizeof(char)), *cp = line_interpreted;
        for (char *c = line; *c != '\0'; ++c) {
            bool matched = false;
            for (size_t i = 0; i < 9; ++i) {
                char *p = strstr(c, digits_strings[i]);
                if (p == c) {
                    *cp = '1' + (char)i;
                    cp++;
                    matched = true;
                }
            }

            if (not matched) {
                *cp = *c;
                cp++;
            }
        }

        for (size_t i = 0; i < 3; ++i) {
            digits[i] = '\0';
        }
        for (char *c = line_interpreted; *c != '\0'; ++c) {
            if (*c >= '0' and *c <= '9') {
                if (digits[0] == '\0') {
                    digits[0] = *c;
                } else {
                    digits[1] = *c;
                }
            }
        }

        if (digits[1] == '\0') {
            digits[1] = digits[0];
        }

        errno = 0;
        int_array_append(&calibrations_all_digits, (int)strtol(digits, NULL, 10));
        if (errno != 0) {
            fprintf(stderr, "could not convert string '%s' to long: %s\n", digits, strerror(errno));
            return EXIT_FAILURE;
        }
    }

    printf("--- Part One ---\n");
    printf("Consider your entire calibration document. What is the sum of all of the calibration values?\n");

    long sum = 0;
    for (size_t i = 0; i < calibrations.len; ++i) {
        sum += calibrations.data[i];
    }
    printf("The sum of all of the calibration values is %ld\n", sum);

    printf("--- Part Two ---\n");
    printf("Your calculation isn't quite right. It looks like some of the digits are actually spelled out with "
           "letters: one, two, three, four, five, six, seven, eight, and nine also count as valid 'digits'. Equipped "
           "with this new information, you now need to find the real first and last digit on each line.\n");
    printf("What is the sum of all of the calibration values?\n");

    sum = 0;
    for (size_t i = 0; i < calibrations_all_digits.len; ++i) {
        sum += calibrations_all_digits.data[i];
    }
    printf("The sum of all of the calibration values is %ld\n", sum);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
