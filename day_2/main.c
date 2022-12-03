#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "helpers.h"

int usage(const char *name) {
    printf("usage: %s input\n", name);
    printf("\tinput: path to input file, '-' to use stdin\n");
    return EXIT_FAILURE;
}

typedef struct round {
    char opponent, hint;
} round;

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
    if (!fptr) {
        fprintf(stderr, "could not open %s: %s\n", input, strerror(errno));
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    size_t rounds_length = 1;
    round *rounds = calloc(rounds_length, sizeof(round));
    if (!rounds) {
        fprintf(stderr, "could not allocate %ld bytes: %s\n", rounds_length * sizeof(round), strerror(errno));
        return EXIT_FAILURE;
    }

    size_t current = 0;
    while ((nread = getline(&line, &len, fptr)) != -1) {
        if (strequ(line, "\n")) {
            continue;
        }

        if (!(current + 1 < rounds_length)) {
            rounds_length *= 2;
            rounds = realloc(rounds, rounds_length * sizeof(round));
            if (!rounds) {
                fprintf(stderr, "could not reallocate %ld bytes: %s\n", rounds_length * sizeof(round), strerror(errno));
                return EXIT_FAILURE;
            }
        }

        round r;
        if (sscanf(line, "%c %c\n", &r.opponent, &r.hint) != 2) {
            fprintf(stderr, "could not read input from line '%s': %s", line, strerror(errno));
            return EXIT_FAILURE;
        }

        rounds[current++] = r;
    }
    rounds_length = current;

    {
        printf("--- Part One ---\n");
        printf("What would your total score be if everything goes exactly according to your strategy guide?\n");

        int total_score = 0;
        for (size_t i = 0; i < rounds_length; ++i) {
            int score = score_round((hand)(rounds[i].hint - 'X' + 'A'), (hand)(rounds[i].opponent));
            total_score += score + (int)(rounds[i].hint - 'X' + 1);
        }

        printf("The total score if everything goes exactly according to the strategy guide would be %d\n", total_score);
    }

    {
        printf("--- Part Two ---\n");
        printf(
            "Following the Elf's instructions for the second column, what would your total score be if everything goes "
            "exactly according to your strategy guide?\n");

        int total_score = 0;
        for (size_t i = 0; i < rounds_length; ++i) {
            hand player = resolve((hand)rounds[i].opponent, (target)rounds[i].hint);
            total_score += score_round(player, (hand)(rounds[i].opponent)) + (int)(player - 'A' + 1);
        }

        printf("The total score if everything goes exactly according to the strategy guide would be %d\n", total_score);
    }

    free(rounds);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
