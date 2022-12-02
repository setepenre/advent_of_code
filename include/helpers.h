#ifndef HELPERS_H
#define HELPERS_H

#include <stdbool.h>
#include <string.h>

extern inline bool strequ(const char *s1, const char *s2) { return strcmp(s1, s2) == 0 ? true : false; }

#endif
