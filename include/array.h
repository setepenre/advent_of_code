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
    extern inline void typename##_free(typename *array) {                                                              \
        if (array->data) {                                                                                             \
            free(array->data);                                                                                         \
        }                                                                                                              \
    }
