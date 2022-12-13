#pragma once

#define STACK(type, typename)                                                                                          \
    typedef struct {                                                                                                   \
        size_t cap, len;                                                                                               \
        type *data;                                                                                                    \
    } typename;                                                                                                        \
    typename *typename##_push(typename *stack, const type value) {                                                     \
        if (!stack->cap || !stack->data) {                                                                             \
            stack->cap = 1;                                                                                            \
            stack->data = calloc(stack->cap, sizeof(type));                                                            \
        }                                                                                                              \
        if (!(stack->len + 1 < stack->cap)) {                                                                          \
            stack->cap *= 2;                                                                                           \
            stack->data = realloc(stack->data, stack->cap * sizeof(type));                                             \
        }                                                                                                              \
        if (!stack->data) {                                                                                            \
            return NULL;                                                                                               \
        }                                                                                                              \
        stack->data[stack->len++] = value;                                                                             \
        return stack;                                                                                                  \
    }                                                                                                                  \
    extern inline type typename##_pop(typename *stack) { return stack->data[--stack->len]; }                           \
    extern inline type typename##_top(const typename *stack) { return stack->data[stack->len - 1]; }                   \
    extern inline void typename##_free(typename *stack) {                                                              \
        if (stack->data) {                                                                                             \
            free(stack->data);                                                                                         \
        }                                                                                                              \
    }
