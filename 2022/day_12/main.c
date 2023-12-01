#include <errno.h>
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "helpers.h"
#include "vec2i.h"

ARRAY(int, int_array)
ARRAY(vec2i_t, vec2i_array)

typedef struct {
    size_t width, height;
    int_array heights;
} heightmap_t;

int height_at(const heightmap_t *heightmap, vec2i_t p) {
    return heightmap->heights.data[(size_t)p.x + (size_t)p.y * heightmap->width];
}

bool is_traversable(const heightmap_t *heightmap, vec2i_t from, vec2i_t to) {
    if (not(to.x >= 0 and to.x < (int)heightmap->width and to.y >= 0 and to.y < (int)heightmap->height)) {
        return false;
    }

    return height_at(heightmap, from) + 1 >= height_at(heightmap, to);
}

typedef struct node_t node_t;
struct node_t {
    vec2i_t p;
    int g, h;
    node_t *parent;
};

static inline int f_cost(node_t *node) { return node->g * node->h; }

ARRAY(node_t *, node_array)

static inline void add_node(node_array *nodes, node_t *node) { node_array_append(nodes, node); }

void remove_node(node_array *nodes, const node_t *node) {
    for (size_t i = 0; i < nodes->len; ++i) {
        if (nodes->data[i] == node) {
            memmove(nodes->data + i, nodes->data + i + 1, (nodes->len - 1 - i) * sizeof(node_t *));
            nodes->len--;
        }
    }
}

node_t *get_node(const node_array *nodes, vec2i_t p) {
    for (size_t i = 0; i < nodes->len; ++i) {
        if (vec2i_equ(nodes->data[i]->p, p)) {
            return nodes->data[i];
        }
    }
    return NULL;
}

static inline bool is_in(const node_array *nodes, vec2i_t p) {
    for (size_t i = 0; i < nodes->len; ++i) {
        if (vec2i_equ(nodes->data[i]->p, p)) {
            return true;
        }
    }
    return false;
}

node_t *lowest_cost(const node_array *nodes) {
    int min = -1;
    size_t index = 0;
    for (size_t i = 0; i < nodes->len; ++i) {
        if (f_cost(nodes->data[i]) < min) {
            min = f_cost(nodes->data[i]);
            index = i;
        }
    }
    return nodes->data[index];
}

static inline int distance(vec2i_t a, vec2i_t b) { return abs(a.x - b.x) + abs(a.y - b.y); }

int a_star(const heightmap_t *heightmap, vec2i_t start_p, vec2i_t end_p) {
    node_t *start = malloc(sizeof(node_t));
    start->p = start_p;
    start->g = 0;
    start->h = 0;
    start->parent = NULL;

    node_array open = {0, 0, NULL}, closed = {0, 0, NULL};
    add_node(&closed, start);

    node_t *current = start;
    while (not vec2i_equ(current->p, end_p)) {
        vec2i_t neighbors[4] = {vec2i_add(current->p, vec2i(0, +1)), vec2i_add(current->p, vec2i(+1, 0)),
                                vec2i_add(current->p, vec2i(0, -1)), vec2i_add(current->p, vec2i(-1, 0))};
        for (size_t i = 0; i < 4; ++i) {
            if (not is_traversable(heightmap, current->p, neighbors[i]) or is_in(&closed, neighbors[i])) {
                continue;
            }

            node_t *neighbor = get_node(&open, neighbors[i]);
            if (not neighbor) {
                neighbor = malloc(sizeof(node_t));
                neighbor->p = neighbors[i];
                neighbor->g = distance(neighbor->p, current->p);
                neighbor->h = distance(neighbor->p, end_p);
                neighbor->parent = current;
            } else if (current->g + 1 < neighbor->g) {
                neighbor->g = current->g + 1;
                neighbor->parent = current;
            }

            if (not is_in(&open, neighbor->p)) {
                add_node(&open, neighbor);
            }
        }

        if (open.len == 0) {
            break;
        }
        current = lowest_cost(&open);
        remove_node(&open, current);
        add_node(&closed, current);
    }

    int count = 0;
    if (open.len == 0) {
        count = -1;
    } else {
        while (current != start) {
            current = current->parent;
            count++;
        }
    }

    for (size_t i = 0; i < closed.len; ++i) {
        free(closed.data[closed.len - 1 - i]);
    }
    node_array_free(&closed);

    for (size_t i = 0; i < open.len; ++i) {
        free(open.data[open.len - 1 - i]);
    }
    node_array_free(&open);

    return count;
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
    if (!fptr) {
        fprintf(stderr, "could not open %s: %s\n", input, strerror(errno));
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t len = 0;

    vec2i_t start_p = {0, 0}, end_p = {0, 0};
    heightmap_t heightmap = {0, 0, {0, 0, NULL}};
    while (getline(&line, &len, fptr) != -1) {
        if (strequ(line, "\n")) {
            continue;
        }

        heightmap.height++;
        if (heightmap.width == 0) {
            heightmap.width = strlen(line) - 1;
        }
        for (char *c = line; *c != '\n'; ++c) {
            if (not(*c >= 'a' or *c <= 'z' or *c == 'S' or *c == 'E')) {
                continue;
            }
            int_array_append(&heightmap.heights, *c == 'S' ? 'a' - 'a' : *c == 'E' ? 'z' - 'a' : *c - 'a');
            if (*c == 'S') {
                start_p = vec2i((int)(heightmap.heights.len - 1) % (int)heightmap.width,
                                (int)(heightmap.heights.len - 1) / (int)heightmap.width);
            }
            if (*c == 'E') {
                end_p = vec2i((int)(heightmap.heights.len - 1) % (int)heightmap.width,
                              (int)(heightmap.heights.len - 1) / (int)heightmap.width);
            }
        }
    }

    {
        printf("--- Part One ---\n");
        printf("What is the fewest steps required to move from your current position to the location that should get "
               "the best signal?\n");

        int count = a_star(&heightmap, start_p, end_p);

        printf("The fewest steps required to move from your current position to the location is %d steps\n", count);
    }

    {
        printf("--- Part Two ---\n");
        printf("What is the fewest steps required to move starting from any square with elevation a to the location "
               "that should get the best signal?\n");

        vec2i_array lowest_elevations = {0, 0, NULL};
        for (int y = 0; y < (int)heightmap.height; ++y) {
            for (int x = 0; x < (int)heightmap.width; ++x) {
                if ((char)height_at(&heightmap, vec2i(x, y)) + 'a' == 'a') {
                    vec2i_array_append(&lowest_elevations, vec2i(x, y));
                }
            }
        }

        int minimum_count = 0;
        for (size_t i = 0; i < lowest_elevations.len; ++i) {
            int count = a_star(&heightmap, lowest_elevations.data[i], end_p);
            if (count == -1) {
                continue;
            }

            if (minimum_count == 0 or count < minimum_count) {
                minimum_count = count;
            }
            printf("search %2.0f%% complete\r", (float)i / (float)(lowest_elevations.len - 1) * 100.0);
            fflush(stdout);
        }

        vec2i_array_free(&lowest_elevations);

        printf("The fewest steps required to move from any square with elevation a to the location is %d steps\n",
               minimum_count);
    }
    int_array_free(&heightmap.heights);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
