#include <errno.h>
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "helpers.h"

typedef struct node_t node_t;
struct node_t {
    char *name;
    size_t size;
    node_t *parent;
    size_t children_count;
    node_t *children;
};

ARRAY(node_t *, node_t_ptr_array)

static inline const char *fmt_node(node_t *node) {
    return node->size == 0 ? "%s %s (dir, size=%zu)\n" : "%s %s (file, size=%zu)\n";
}

size_t compute_tree_size(node_t *node) {
    if (not node) {
        return 0;
    }

    size_t size = node->size;
    for (node_t *cursor = node->children; cursor != NULL and cursor < node->children + node->children_count; ++cursor) {
        size += compute_tree_size(cursor);
    }

    return size;
}

bool part_one_predicate(node_t *node, void *data) {
    (void)data;
    return node->size == 0 and compute_tree_size(node) <= 100000;
}

bool part_two_predicate(node_t *node, void *data) {
    size_t minimal_size = *(size_t *)data;
    return node->size == 0 and compute_tree_size(node) > minimal_size;
}

node_t_ptr_array filter_tree(node_t *node, bool (*f)(node_t *, void *), void *data) {
    node_t_ptr_array acc = {0, 0, NULL};
    if (not node) {
        return acc;
    }

    if (f(node, data)) {
        node_t_ptr_array_append(&acc, node);
    }

    for (node_t *cursor = node->children; cursor != NULL and cursor < node->children + node->children_count; ++cursor) {
        node_t_ptr_array arr = filter_tree(cursor, f, data);
        node_t_ptr_array_concat(&acc, &arr);
        node_t_ptr_array_free(&arr);
    }

    return acc;
}

void print_tree(node_t *node, const char *prefix) {
    if (not node) {
        return;
    }

    printf(fmt_node(node), prefix, node->name, node->size);

    char *subprefix = calloc(strlen(prefix) + 2 + 1, sizeof(char));
    sprintf(subprefix, "  %s", prefix);

    for (node_t *cursor = node->children; cursor != NULL and cursor < node->children + node->children_count; ++cursor) {
        print_tree(cursor, subprefix);
    }

    free(subprefix);
}

void delete_tree(node_t *node) {
    if (not node) {
        return;
    }

    for (node_t *cursor = node->children; cursor != NULL and cursor < node->children + node->children_count; ++cursor) {
        delete_tree(cursor);
        free(cursor->name);
    }

    if (node->children) {
        free(node->children);
    }
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
    if (not fptr) {
        fprintf(stderr, "could not open %s: %s\n", input, strerror(errno));
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t len = 0;

    node_t root = {"/", 0, NULL, 0, NULL};
    node_t *current = &root;
    while (getline(&line, &len, fptr) != -1) {
        if (line[0] != '$') {
            continue;
        }

        if (strnequ(line + 2, "cd", 2)) {
            const size_t arg_len = (strlen(line + 2) - 1) - (size_t)(strchr(line + 2, ' ') - (line + 2));
            char arg[arg_len];
            sscanf(line, "$ cd %s\n", arg);

            if (strequ(arg, "..")) {
                if (current->parent) {
                    current = current->parent;
                }
                continue;
            }

            for (node_t *cursor = current->children;
                 cursor != NULL and cursor < current->children + current->children_count; ++cursor) {
                if (strequ(cursor->name, arg)) {
                    current = cursor;
                    break;
                }
            }
        }

        if (strnequ(line + 2, "ls", 2) and not current->children) {
            size_t children_count = 0;
            node_t *children = NULL;
            while (getline(&line, &len, fptr) != -1 and line[0] != '$') {
                const size_t name_length = (size_t)((line + strlen(line) - 1) - strchr(line, ' '));
                char name[name_length];
                size_t size = 0;

                if (sscanf(line, "%zu %s\n", &size, name) != 2) {
                    sscanf(line, "dir %s\n", name);
                    size = 0;
                }

                node_t node = {calloc(strlen(name) + 1, sizeof(char)), size, current, 0, NULL};
                if (not node.name) {
                    fprintf(stderr, "could not allocate %ld bytes to store node.name: %s\n",
                            strlen(name) * sizeof(char), strerror(errno));
                    return EXIT_FAILURE;
                }
                sprintf(node.name, "%s", name);
                children = not children ? calloc(++children_count, sizeof(node_t))
                                        : realloc(children, (++children_count) * sizeof(node_t));
                if (not children) {
                    fprintf(stderr, "could not allocate %ld bytes to grow children array: %s\n",
                            children_count * sizeof(node_t), strerror(errno));
                }
                children[children_count - 1] = node;
            }

            current->children_count = children_count;
            current->children = children;

            fseek(fptr, -(long)strlen(line), SEEK_CUR);
        }
    }
    current = &root;

    /* print_tree(&root, "-"); */

    {
        printf("--- Part One ---\n");
        printf("Find all of the directories with a total size of at most 100000. What is the sum of the total sizes of "
               "those directories?\n");

        size_t total_size = 0;
        node_t_ptr_array directories = filter_tree(&root, part_one_predicate, NULL);
        for (size_t i = 0; i < directories.len; ++i) {
            total_size += compute_tree_size(directories.data[i]);
        }

        printf("The sum of the total sizes of those directories is %zu\n", total_size);

        node_t_ptr_array_free(&directories);
    }

    {
        printf("--- Part Two ---\n");
        printf("Find the smallest directory that, if deleted, would free up enough space on the filesystem to run the "
               "update. What is the total size of that directory?\n");

        size_t minimal_size = 30000000 - (70000000 - compute_tree_size(&root));
        node_t_ptr_array directories = filter_tree(&root, part_two_predicate, (void *)&minimal_size);
        if (not(directories.len > 0)) {
            fprintf(stderr, "could not find any directory matching predicate\n");
            return EXIT_FAILURE;
        }
        node_t *smallest = directories.data[0];
        for (size_t i = 1; i < directories.len; ++i) {
            if (compute_tree_size(smallest) > compute_tree_size(directories.data[i])) {
                smallest = directories.data[i];
            }
        }

        printf("The sum of the total sizes of those directories is %zu\n", compute_tree_size(smallest));

        node_t_ptr_array_free(&directories);
    }

    delete_tree(&root);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
