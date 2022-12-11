#define DEQUEUE(type, typename)                                                                                        \
    typedef struct {                                                                                                   \
        size_t cap, len;                                                                                               \
        type *buffer, *front;                                                                                          \
    } typename;                                                                                                        \
    typename *typename##_push_back(typename *dequeue, type value) {                                                    \
        if (!dequeue->cap || !dequeue->buffer) {                                                                       \
            dequeue->cap = 1;                                                                                          \
            dequeue->buffer = calloc(dequeue->cap, sizeof(type));                                                      \
            dequeue->front = dequeue->buffer;                                                                          \
        }                                                                                                              \
        if (!(dequeue->len + 1 < dequeue->cap)) {                                                                      \
            if (dequeue->buffer != dequeue->front) {                                                                   \
                memmove(dequeue->buffer, dequeue->front, dequeue->cap * sizeof(type));                                 \
            }                                                                                                          \
            dequeue->cap *= 2;                                                                                         \
            dequeue->buffer = realloc(dequeue->buffer, dequeue->cap * sizeof(type));                                   \
            dequeue->front = dequeue->buffer;                                                                          \
        }                                                                                                              \
        if (!dequeue->buffer) {                                                                                        \
            return NULL;                                                                                               \
        }                                                                                                              \
        dequeue->buffer[dequeue->len++] = value;                                                                       \
        return dequeue;                                                                                                \
    }                                                                                                                  \
    type typename##_pop_front(typename *dequeue) {                                                                     \
        type front = dequeue->front[0];                                                                                \
        dequeue->cap--;                                                                                                \
        dequeue->len--;                                                                                                \
        dequeue->front++;                                                                                              \
        if (dequeue->len == 0) {                                                                                       \
            dequeue->cap = 0;                                                                                          \
            dequeue->front = NULL;                                                                                     \
            free(dequeue->buffer);                                                                                     \
            dequeue->buffer = NULL;                                                                                    \
        }                                                                                                              \
        return front;                                                                                                  \
    }                                                                                                                  \
    extern inline void typename##_free(typename *dequeue) {                                                            \
        if (dequeue->buffer) {                                                                                         \
            free(dequeue->buffer);                                                                                     \
        }                                                                                                              \
    }
