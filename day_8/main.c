#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "helpers.h"

ARRAY(int, int_array)
ARRAY(size_t, size_t_array)

typedef struct {
    size_t up, right, down, left;
} view_t;

static inline size_t scenic_score(view_t view) { return view.up * view.right * view.down * view.left; }

ARRAY(view_t, view_t_array)

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

    size_t rows = 0, cols = 0;
    int_array heights = {0, 0, NULL};
    while (getline(&line, &len, fptr) != -1) {
        if (strequ(line, "\n")) {
            continue;
        }

        if (!(strlen(line) - 1 > 0)) {
            continue;
        }

        if (cols == 0) {
            cols = strlen(line) - 1;
        }

        for (char *c = line; *c != '\n'; ++c) {
            if (*c < '0' || *c > '9') {
                continue;
            }
            int_array_append(&heights, *c - '0');
        }

        ++rows;
    }

    {
        printf("--- Part One ---\n");
        printf("Consider your map; how many trees are visible from outside the grid?\n");

        size_t_array visibility = {0, 0, NULL};
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                size_t_array_append(&visibility, (i == 0 || i == rows - 1 || j == 0 || j == cols - 1) ? 1 : 0);
            }
        }

        for (size_t i = 1; i < rows - 1; ++i) {
            int highest = heights.data[0 + i * cols];
            for (size_t j = 1; j < cols - 1; ++j) {
                int height = heights.data[j + i * cols];
                if (height > highest) {
                    visibility.data[j + i * cols] = 1;
                    highest = height;
                }

                if (height == 9) {
                    break;
                }
            }
        }

        for (size_t i = 1; i < rows - 1; ++i) {
            int highest = heights.data[(cols - 1 - 0) + i * cols];
            for (size_t j = 1; j < cols - 1; ++j) {
                int height = heights.data[(cols - 1 - j) + i * cols];
                if (height > highest) {
                    visibility.data[(cols - 1 - j) + i * cols] = 1;
                    highest = height;
                }

                if (height == 9) {
                    break;
                }
            }
        }

        for (size_t j = 1; j < cols - 1; ++j) {
            int highest = heights.data[j + 0 * cols];
            for (size_t i = 1; i < rows - 1; ++i) {
                int height = heights.data[j + i * cols];
                if (height > highest) {
                    visibility.data[j + i * cols] = 1;
                    highest = height;
                }

                if (height == 9) {
                    break;
                }
            }
        }

        for (size_t j = 1; j < cols - 1; ++j) {
            int highest = heights.data[j + (rows - 1 - 0) * cols];
            for (size_t i = 1; i < rows - 1; ++i) {
                int height = heights.data[j + (rows - 1 - i) * cols];
                if (height > highest) {
                    visibility.data[j + (rows - 1 - i) * cols] = 1;
                    highest = height;
                }

                if (height == 9) {
                    break;
                }
            }
        }

        size_t visible = 0;
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                visible += visibility.data[j + i * cols];
            }
        }
        printf("%zu trees are visible from outside the grid.\n", visible);

        size_t_array_free(&visibility);
    }

    {
        printf("--- Part Two ---\n");
        printf("Consider each tree on your map. What is the highest scenic score possible for any tree?\n");

        view_t_array views = {0, 0, NULL};
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                int height = heights.data[j + i * cols];
                view_t view = {0, 0, 0, 0};

                for (int k = (int)i - 1; k >= 0; --k) {
                    view.up++;
                    if (heights.data[j + (size_t)k * cols] >= height) {
                        break;
                    }
                }

                for (int k = (int)j + 1; k < (int)cols; ++k) {
                    view.right++;
                    if (heights.data[(size_t)k + i * cols] >= height) {
                        break;
                    }
                }

                for (int k = (int)i + 1; k < (int)rows; ++k) {
                    view.down++;
                    if (heights.data[j + (size_t)k * cols] >= height) {
                        break;
                    }
                }

                for (int k = (int)j - 1; k >= 0; --k) {
                    view.left++;
                    if (heights.data[(size_t)k + i * cols] >= height) {
                        break;
                    }
                }

                view_t_array_append(&views, view);
            }
        }

        size_t score, highest = 0;
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                if ((score = scenic_score(views.data[j + i * cols])) > highest) {
                    highest = score;
                }
            }
        }

        printf("The highest scenic score possible is %zu\n", highest);

        view_t_array_free(&views);
    }

    int_array_free(&heights);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
