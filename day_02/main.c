#include <errno.h>
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "helpers.h"

int usage(const char *name) {
    printf("usage: %s input\n", name);
    printf("\tinput: path to input file, '-' to use stdin\n");
    return EXIT_FAILURE;
}

typedef struct {
    char opponent, hint;
} round;

ARRAY(round, round_array)

typedef enum hand { ROCK = 'A', PAPER = 'B', SCISSOR = 'C' } hand;
typedef enum target { LOSE = 'X', DRAW = 'Y', WIN = 'Z' } target;

int dnorm(int a, int b) { return (a - b) != 0 ? (a - b) / abs(a - b) : 0; }

int score_round(hand left, hand right) {
    return (abs((int)(left - right)) > 1 ? -1 : 1) * 3 * dnorm((int)left, (int)right) + 3;
}

hand resolve(hand left, target right) { return ((3 + (left - 'A') + (right - 'Y'))) % 3 + 'A'; }

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

    round_array rounds = {0, 0, NULL};
    while (getline(&line, &len, fptr) != -1) {
        if (strequ(line, "\n")) {
            continue;
        }

        round r;
        if (sscanf(line, "%c %c\n", &r.opponent, &r.hint) != 2) {
            fprintf(stderr, "could not read input from line '%s': %s", line, strerror(errno));
            return EXIT_FAILURE;
        }

        if (not round_array_append(&rounds, r)) {
            fprintf(stderr, "could not reallocate %ld bytes: %s\n", rounds.cap * sizeof(round), strerror(errno));
            return EXIT_FAILURE;
        }
    }

    {
        printf("--- Part One ---\n");
        printf("What would your total score be if everything goes exactly according to your strategy guide?\n");

        int total_score = 0;
        for (size_t i = 0; i < rounds.len; ++i) {
            int score = score_round((hand)(rounds.data[i].hint - 'X' + 'A'), (hand)(rounds.data[i].opponent));
            total_score += score + (int)(rounds.data[i].hint - 'X' + 1);
        }

        printf("The total score if everything goes exactly according to the strategy guide would be %d\n", total_score);
    }

    {
        printf("--- Part Two ---\n");
        printf(
            "Following the Elf's instructions for the second column, what would your total score be if everything goes "
            "exactly according to your strategy guide?\n");

        int total_score = 0;
        for (size_t i = 0; i < rounds.len; ++i) {
            hand player = resolve((hand)rounds.data[i].opponent, (target)rounds.data[i].hint);
            total_score += score_round(player, (hand)(rounds.data[i].opponent)) + (int)(player - 'A' + 1);
        }

        printf("The total score if everything goes exactly according to the strategy guide would be %d\n", total_score);
    }

    round_array_free(&rounds);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
