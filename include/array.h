#pragma once

#define ARRAY(type, typename)                                                                                          \
    typedef struct {                                                                                                   \
        size_t cap, len;                                                                                               \
        type *data;                                                                                                    \
    } typename;                                                                                                        \
    typename *typename##_append(typename *array, type value) {                                                         \
        if (!array->cap || !array->data) {                                                                             \
            array->cap = 1;                                                                                            \
            array->data = calloc(array->cap, sizeof(type));                                                            \
        }                                                                                                              \
        if (!(array->len + 1 < array->cap)) {                                                                          \
            array->cap *= 2;                                                                                           \
            array->data = realloc(array->data, array->cap * sizeof(type));                                             \
        }                                                                                                              \
        if (!array->data) {                                                                                            \
            return NULL;                                                                                               \
        }                                                                                                              \
        array->data[array->len++] = value;                                                                             \
        return array;                                                                                                  \
    }                                                                                                                  \
    typename *typename##_concat(typename *dst, const typename *src) {                                                  \
        if (!src->cap || !src->data) {                                                                                 \
            return dst;                                                                                                \
        }                                                                                                              \
        if (!dst->cap || !dst->data) {                                                                                 \
            dst->cap = src->cap;                                                                                       \
            dst->data = calloc(dst->cap, sizeof(type));                                                                \
        }                                                                                                              \
        while (!(dst->len + src->len < dst->cap)) {                                                                    \
            dst->cap *= 2;                                                                                             \
            dst->data = realloc(dst->data, dst->cap * sizeof(type));                                                   \
        }                                                                                                              \
        if (!dst->data) {                                                                                              \
            return NULL;                                                                                               \
        }                                                                                                              \
        memcpy(dst->data + dst->len, src->data, src->len * sizeof(type));                                              \
        dst->len += src->len;                                                                                          \
        return dst;                                                                                                    \
    }                                                                                                                  \
    extern inline void typename##_free(typename *array) {                                                              \
        if (array->data) {                                                                                             \
            free(array->data);                                                                                         \
        }                                                                                                              \
    }
