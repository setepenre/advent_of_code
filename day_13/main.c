#include <errno.h>
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "helpers.h"

typedef struct packet_t packet_t;
struct packet_t {
    packet_t *parent;
    packet_t *next;
    packet_t *child;
    int *value;
};

packet_t *new_packet(packet_t *parent) {
    packet_t *node = malloc(sizeof(packet_t));
    node->parent = parent;
    node->next = NULL;
    node->child = NULL;
    node->value = NULL;
    return node;
}

packet_t *divider_packet(int value) {
    packet_t *divider_packet = new_packet(NULL);
    divider_packet->child = new_packet(divider_packet);
    divider_packet->child->child = new_packet(divider_packet);
    divider_packet->child->child->value = malloc(sizeof(int));
    *divider_packet->child->child->value = value;
    return divider_packet;
}

ARRAY(packet_t *, packet_array)

void print_node(const packet_t *node) {
    if (not node) {
        return;
    }

    if (node->child) {
        printf("[");
        print_node(node->child);
        printf("]");
    }

    if (node->value) {
        printf("%d", *node->value);
    }

    if (node->next) {
        printf(", ");
        print_node(node->next);
    }
}

void delete_nodes(packet_t *node) {
    if (not node) {
        return;
    }

    if (node->value) {
        free(node->value);
    }

    delete_nodes(node->child);
    delete_nodes(node->next);

    free(node);
}

typedef enum { RIGHT, NOT_RIGHT, UNK } cmp;
cmp packet_cmp(packet_t *left, packet_t *right) {
    if (not left and right) {
        return RIGHT;
    }

    if (left and not right) {
        return NOT_RIGHT;
    }

    if (not left and not right) {
        return UNK;
    }

    if (left->value and right->value) {
        if (*left->value < *right->value) {
            return RIGHT;
        } else if (*left->value > *right->value) {
            return NOT_RIGHT;
        } else {
            return packet_cmp(left->next, right->next);
        }
    }

    if (left->value and right->child) {
        left->child = new_packet(left);
        left->child->value = left->value;
        left->value = NULL;
    } else if (left->child and right->value) {
        right->child = new_packet(right);
        right->child->value = right->value;
        right->value = NULL;
    }

    if (not left->value and right->value) {
        return RIGHT;
    } else if (left->value and not right->value) {
        return NOT_RIGHT;
    }

    cmp child = packet_cmp(left->child, right->child);
    if (child != UNK) {
        return child;
    }

    return packet_cmp(left->next, right->next);
}

void sort_packet_array(packet_array *array) {
    for (size_t i = 0; i < array->len - 1; ++i) {
        for (size_t j = 0; j < array->len - i - 1; ++j) {
            if (packet_cmp(array->data[j], array->data[j + 1]) == NOT_RIGHT) {
                packet_t *tmp = array->data[j];
                array->data[j] = array->data[j + 1];
                array->data[j + 1] = tmp;
            }
        }
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
    if (!fptr) {
        fprintf(stderr, "could not open %s: %s\n", input, strerror(errno));
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t len = 0;

    packet_array packets = {0, 0, NULL};
    while (getline(&line, &len, fptr) != -1) {
        if (strequ(line, "\n")) {
            continue;
        }

        packet_t *current = new_packet(NULL);
        for (char *c = line; *c != '\n'; ++c) {
            switch (*c) {
            case '[':
                current->child = new_packet(current);
                current = current->child;
                break;
            case ',':
                current->next = new_packet(current->parent);
                current = current->next;
                break;
            case ']':
                current = current->parent;
                break;
            default:
                if (*c >= '0' and *c <= '9') {
                    current->value = malloc(sizeof(int));
                    if (*c == '1' and *(c + 1) == '0') {
                        *current->value = 10;
                        ++c;
                    } else {
                        *current->value = *c - '0';
                    }
                }
                break;
            }
        }

        packet_array_append(&packets, current);
    }

    {
        printf("--- Part One ---\n");
        printf("Determine which pairs of packets are already in the right order. What is the sum of the indices of "
               "those pairs?\n");

        size_t sum = 0;
        for (size_t i = 0; i < packets.len / 2; ++i) {
            if (packet_cmp(packets.data[2 * i], packets.data[2 * i + 1]) == RIGHT) {
                sum += (i + 1);
            }
        }

        printf("The sum of the indices of the pairs of packets already in the right order is %zu.\n", sum);
    }

    {
        printf("--- Part Two ---\n");
        printf(
            "Organize all of the packets into the correct order. What is the decoder key for the distress signal?\n");

        packet_t *probe_2 = divider_packet(2);
        packet_array_append(&packets, probe_2);
        packet_t *probe_6 = divider_packet(6);
        packet_array_append(&packets, probe_6);

        packet_array sorted = {packets.cap, packets.len, packets.data};
        sort_packet_array(&sorted);

        size_t key = 1;
        for (size_t i = 0; i < sorted.len; ++i) {
            if (sorted.data[i] == probe_2 or sorted.data[i] == probe_6) {
                key *= (i + 1);
            }
        }

        printf("The decoder key for the distress signal is: %zu.\n", key);
    }

    for (size_t i = 0; i < packets.len; ++i) {
        delete_nodes(packets.data[i]);
    }
    packet_array_free(&packets);

    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
