#include <errno.h>
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>

#include <ncurses.h>

#include "array.h"
#include "helpers.h"
#include "vec2i.h"

ARRAY(vec2i_t, vec2i_array)

typedef enum { SAND_SOURCE = '+', ROCK = '#', AIR = '.', SAND = 'o' } point_type;
typedef struct {
    vec2i_t p;
    point_type t;
} point_t;

ARRAY(point_t, point_array)

typedef struct {
    vec2i_t min, max;
    vec2i_t source;
    point_array points;
} map_t;

bool in(const map_t *m, vec2i_t p) {
    return (p.x >= m->min.x and p.x <= m->max.x) and (p.y >= m->min.y and p.y <= m->max.y);
}

point_t *at(const map_t *m, vec2i_t p) {
    if (not in(m, p)) {
        return NULL;
    }

    int i = p.y - m->min.y, j = p.x - m->min.x, w = abs(m->max.x - m->min.x) + 1;
    return &m->points.data[j + i * w];
}

void resize(map_t *m, vec2i_t min, vec2i_t max) {
    point_array previous_points = m->points, points = {0, 0, NULL};

    m->min = min;
    m->max = max;
    for (int y = m->min.y; y < m->max.y + 1; ++y) {
        for (int x = m->min.x; x < m->max.x + 1; ++x) {
            vec2i_t p = vec2i(x, y);
            point_t point = {p, vec2i_equ(p, m->source) ? SAND_SOURCE : y == m->max.y ? ROCK : AIR};
            point_array_append(&points, point);
        }
    }
    m->points = points;

    for (size_t i = 0; i < previous_points.len; ++i) {
        point_t *point = at(m, previous_points.data[i].p);
        if (not point) {
            continue;
        }
        point->t = previous_points.data[i].t;
    }

    point_array_free(&previous_points);
}

void ncurses_draw(const map_t *m, vec2i_t offset) {
    clear();

    size_t sand_count = 0;
    vec2i_t start_win = {(COLS - (m->max.x - m->min.x)) / 2, (LINES - (m->max.y - m->min.y)) / 2};
    for (int y = m->min.y; y < m->max.y + 1; ++y) {
        for (int x = m->min.x; x < m->max.x + 1; ++x) {
            point_t *point = at(m, vec2i(x, y));
            if (point->t == SAND) {
                sand_count++;
                attron(COLOR_PAIR(1));
            }

            switch (point->t) {
            case SAND:
                attron(COLOR_PAIR(1));
                break;
            case ROCK:
                attron(COLOR_PAIR(2));
                break;
            case SAND_SOURCE:
            case AIR:
            default:
                break;
            }

            mvprintw(start_win.y + y - m->min.y + offset.y, start_win.x + x - m->min.x + offset.x, "%c", point->t);

            switch (point->t) {
            case SAND:
                attroff(COLOR_PAIR(1));
                break;
            case ROCK:
                attroff(COLOR_PAIR(2));
                break;
            case SAND_SOURCE:
            case AIR:
            default:
                break;
            }
        }
    }
    mvprintw(start_win.y + m->max.y + 1 + offset.y, start_win.x + offset.x, "%zu units of sand", sand_count);
    refresh();
}

size_t count_sand(const map_t *m) {
    size_t count = 0;
    for (size_t i = 0; i < m->points.len; ++i) {
        if (m->points.data[i].t == SAND) {
            count++;
        }
    }
    return count;
}

typedef struct {
    vec2i_t p;
    bool at_rest;
} sand_t;

static const vec2i_t south = {0, 1}, south_west = {-1, 1}, south_east = {+1, 1};
bool update(map_t *map, sand_t *current) {
    vec2i_t directions[3] = {south, south_west, south_east};

    for (size_t i = 0; i < 3; ++i) {
        vec2i_t candidate = vec2i_add(current->p, directions[i]);
        point_t *point = at(map, candidate);
        if (not point) {
            at(map, current->p)->t = AIR;
            return false;
        }
        if (point->t == AIR) {
            at(map, current->p)->t = not vec2i_equ(current->p, map->source) ? AIR : SAND_SOURCE;
            current->p = candidate;
            current->at_rest = false;
            at(map, current->p)->t = SAND;
            return true;
        }
    }

    current->at_rest = true;
    return true;
}

int usage(const char *name) {
    printf("usage: %s [-d] input\n", name);
    printf("\t-d: toggle to display ncurses animation, absence means no display\n");
    printf("\tinput: path to input file, '-' to use stdin\n");
    return EXIT_FAILURE;
}

int main(int argc, char *argv[]) {
    if (argc - 1 != 1 and argc - 1 != 2) {
        return usage(argv[0]);
    }

    if (argc - 1 == 2 and not strequ(argv[1], "-d")) {
        return usage(argv[0]);
    }
    bool display = argc - 1 == 2 and strequ(argv[1], "-d");

    const char *input = argv[argc - 1];
    FILE *fptr = strequ(input, "-") ? stdin : fopen(input, "r");
    if (!fptr) {
        fprintf(stderr, "could not open %s: %s\n", input, strerror(errno));
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t len = 0;

    vec2i_array rocks = {0, 0, NULL};
    while (getline(&line, &len, fptr) != -1) {
        if (strequ(line, "\n")) {
            continue;
        }

        for (char *s = line; s != NULL; s = strchr(s + 1, '>')) {
            vec2i_t n = {0, 0};
            sscanf(s == line ? s : s + 1, "%d,%d", &n.x, &n.y);
            if (s != line) {
                vec2i_t p = rocks.data[rocks.len - 1],
                        d = n.x - p.x != 0 ? vec2i(sign(n.x - p.x), 0) : vec2i(0, sign(n.y - p.y));
                for (int i = 1; i < abs(p.x - n.x) + abs(p.y - n.y); ++i) {
                    vec2i_array_append(&rocks, vec2i_add(p, vec2i_mul(i, d)));
                }
            }
            vec2i_array_append(&rocks, n);
        }
    }

    vec2i_t sand_source = vec2i(500, 0);
    vec2i_t min = sand_source, max = sand_source;
    for (size_t i = 0; i < rocks.len; ++i) {
        vec2i_t p = rocks.data[i];
        if (p.x < min.x) {
            min.x = p.x;
        }
        if (p.y < min.y) {
            min.y = p.y;
        }
        if (p.x > max.x) {
            max.x = p.x;
        }
        if (p.y > max.y) {
            max.y = p.y;
        }
    }

    const int frametime = 0;
    {
        printf("--- Part One ---\n");
        printf("Using your scan, simulate the falling sand. How many units of sand come to rest before sand starts "
               "flowing "
               "into the abyss below?\n");

        if (display) {
            initscr();
            start_color();

            init_color(COLOR_YELLOW, 900, 800, 600);
            init_pair(1, COLOR_YELLOW, COLOR_BLACK);

            init_color(COLOR_RED, 500, 250, 250);
            init_pair(2, COLOR_RED, COLOR_BLACK);

            cbreak();
            noecho();
            curs_set(0);
            timeout(frametime);
        }

        map_t map = {min, max, sand_source, {0, 0, NULL}};
        for (int y = map.min.y; y < map.max.y + 1; ++y) {
            for (int x = map.min.x; x < map.max.x + 1; ++x) {
                vec2i_t p = vec2i(x, y);
                point_t point = {p, vec2i_equ(p, map.source) ? SAND_SOURCE : AIR};
                point_array_append(&map.points, point);
            }
        }

        for (size_t i = 0; i < rocks.len; ++i) {
            point_t *point = at(&map, rocks.data[i]);
            if (not point) {
                continue;
            }
            point->t = ROCK;
        }

        int ch;
        bool should_update = true, pause = true;
        const vec2i_t sand_start = sand_source;
        sand_t sand = {sand_start, false};
        at(&map, sand.p)->t = SAND;

        vec2i_t offset = {0, 0};
        if (display) {
            ncurses_draw(&map, offset);
        }

        size_t units = 0;
        if (display) {
            while ((ch = getch()) != 'q') {
                switch (ch) {
                case 'w':
                    offset = vec2i_add(offset, vec2i(0, +1));
                    break;
                case 'a':
                    offset = vec2i_add(offset, vec2i(-1, 0));
                    break;
                case 's':
                    offset = vec2i_add(offset, vec2i(0, -1));
                    break;
                case 'd':
                    offset = vec2i_add(offset, vec2i(+1, 0));
                    break;
                case ' ':
                    pause = not pause;
                    break;
                default:
                    break;
                }

                if (should_update and not pause) {
                    should_update = update(&map, &sand);
                    if (sand.at_rest) {
                        sand.p = sand_start;
                        sand.at_rest = false;
                        at(&map, sand.p)->t = SAND;
                        units++;
                    }
                }

                ncurses_draw(&map, offset);
            }
            endwin();
        } else {
            while (should_update) {
                should_update = update(&map, &sand);
                if (sand.at_rest) {
                    sand.p = sand_start;
                    sand.at_rest = false;
                    at(&map, sand.p)->t = SAND;
                    units++;
                }
            }
        }

        point_array_free(&map.points);

        printf("%zu units of sand come to rest before sand starts flowing into the abyss.\n", units);
    }

    {
        printf("--- Part Two ---\n");
        printf("Using your scan, simulate the falling sand until the source of the sand becomes blocked. How many "
               "units of sand come to rest?\n");

        if (display) {
            initscr();
            start_color();

            init_color(COLOR_YELLOW, 900, 800, 600);
            init_pair(1, COLOR_YELLOW, COLOR_BLACK);

            init_color(COLOR_RED, 500, 250, 250);
            init_pair(2, COLOR_RED, COLOR_BLACK);

            cbreak();
            noecho();
            curs_set(0);
            timeout(frametime);
        }

        map_t map = {vec2i(min.x, min.y), vec2i(max.x, max.y + 2), sand_source, {0, 0, NULL}};
        for (int y = map.min.y; y < map.max.y + 1; ++y) {
            for (int x = map.min.x; x < map.max.x + 1; ++x) {
                vec2i_t p = vec2i(x, y);
                point_t point = {p, vec2i_equ(p, map.source) ? SAND_SOURCE : y == map.max.y ? ROCK : AIR};
                point_array_append(&map.points, point);
            }
        }

        for (size_t i = 0; i < rocks.len; ++i) {
            point_t *point = at(&map, rocks.data[i]);
            if (not point) {
                continue;
            }
            point->t = ROCK;
        }

        int ch;
        bool should_update = true, pause = true;
        const vec2i_t sand_start = sand_source;
        sand_t sand = {sand_start, false};
        at(&map, sand.p)->t = SAND;

        vec2i_t offset = {0, 0};
        if (display) {
            ncurses_draw(&map, offset);
        }

        if (display) {
            while ((ch = getch()) != 'q') {
                switch (ch) {
                case 'w':
                    offset = vec2i_add(offset, vec2i(0, +1));
                    break;
                case 'a':
                    offset = vec2i_add(offset, vec2i(-1, 0));
                    break;
                case 's':
                    offset = vec2i_add(offset, vec2i(0, -1));
                    break;
                case 'd':
                    offset = vec2i_add(offset, vec2i(+1, 0));
                    break;
                case ' ':
                    pause = not pause;
                    break;
                default:
                    break;
                }

                if (should_update and not pause) {
                    if (not update(&map, &sand)) {
                        resize(&map, vec2i(map.min.x - 1, map.min.y), vec2i(map.max.x + 1, map.max.y));
                        at(&map, sand.p)->t = SAND;
                    }
                    if (sand.at_rest) {
                        if (vec2i_equ(sand.p, sand_source)) {
                            should_update = false;
                            continue;
                        }

                        sand.p = sand_start;
                        sand.at_rest = false;
                        at(&map, sand.p)->t = SAND;
                    }
                }

                ncurses_draw(&map, offset);
            }
            endwin();
        } else {
            while (should_update) {
                if (not update(&map, &sand)) {
                    resize(&map, vec2i(map.min.x - 1, map.min.y), vec2i(map.max.x + 1, map.max.y));
                    at(&map, sand.p)->t = SAND;
                }
                if (sand.at_rest) {
                    if (vec2i_equ(sand.p, sand_source)) {
                        should_update = false;
                        continue;
                    }
                    sand.p = sand_start;
                    sand.at_rest = false;
                    at(&map, sand.p)->t = SAND;
                }
            }
        }
        size_t units = count_sand(&map);

        point_array_free(&map.points);

        printf("%zu units of sand come to rest before sand starts flowing into the abyss.\n", units);
    }
    vec2i_array_free(&rocks);
    free(line);
    if (fptr != stdin) {
        fclose(fptr);
    }

    return EXIT_SUCCESS;
}
