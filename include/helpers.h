#ifndef HELPERS_H
#define HELPERS_H

#include <stdbool.h>
#include <string.h>

extern inline bool strequ(const char *s1, const char *s2) { return strcmp(s1, s2) == 0 ? true : false; }
extern inline bool strnequ(const char *s1, const char *s2, size_t n) { return strncmp(s1, s2, n) == 0 ? true : false; }

#endif
