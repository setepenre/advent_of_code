#pragma once

#include <stdbool.h>

typedef struct {
    int x, y;
} vec2i_t;

extern inline vec2i_t vec2i(int x, int y) {
    vec2i_t v = {x, y};
    return v;
}

extern inline bool vec2i_equ(vec2i_t a, vec2i_t b) { return a.x == b.x && a.y == b.y; }

extern inline vec2i_t vec2i_add(vec2i_t a, vec2i_t b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}
