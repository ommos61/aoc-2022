
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output

int path_count = 0;
struct point {
    int x;
    int y;
    struct point *next;
};
#define MAX_PATHS 200
struct path {
    struct point *point;
} paths[MAX_PATHS];

void print_path(int path_index) {
    int first = 1;
    struct point *point = paths[path_index].point;
    while (point != NULL) {
        if (first) {
            first = 0;
        } else {
            printf(" -> ");
        }
        printf("%d,%d", point->x, point->y);
        point = point->next;
    }
    printf("\n");
}

void append_path(int paths_index, struct point *point) {
    if (paths[paths_index].point == NULL) {
        paths[paths_index].point = point;
    } else {
        struct point *point_item = paths[paths_index].point;
        while (point_item->next != NULL) {
            point_item = point_item->next;
        }
        point_item->next = point;
    }
}

struct point *get_next_point(char **str) {
    struct point *point = malloc(sizeof(struct point));
    point->next = NULL;

    int val1, val2;
    if (sscanf(*str, "%d,%d", &val1, &val2) == 2) {
        point->x = val1; point->y = val2;
        while (isdigit(**str) || (**str == ',')) {
            *str += 1;
        }
        if (strncmp(*str, " -> ", 4) == 0) {
            *str += 4;
        }
    } else {
        printf("unexpected format in path: '%s'.\n", *str);
    }

    return point;
}

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (strlen(line) != 0) {
            // parse the data
            char *start = line;
            while (strlen(start) != 0) {
                struct point *point = get_next_point(&start);
                append_path(path_count, point);
            }
            path_count++;
        } else if (errno != 0) {
            perror("sscanf");
        } else {
            fprintf(stderr, "Unexpected input format '%s'.\n", line);
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

#define MAP_WIDTH 1000
int minx = INT_MAX, miny = INT_MAX;
int maxx = 0, maxy = 0;
char *map = NULL;
#define GET_MAPPOS(x, y)    (*(map + (MAP_WIDTH * (y)) + (x)))
#define SET_MAPPOS(x, y, c) (*(map + (MAP_WIDTH * (y)) + (x)) = (c))

void print_map(void) {
    for (int y = 0; y <= maxy + 1; y++) {
        for (int x = minx - 1; x <= maxx + 1; x++) {
            printf("%c", GET_MAPPOS(x, y));
        }
        printf("\n");
    }
}

void make_map(void) {
    // determine the min and max for x and y
    for (int i = 0; i < path_count; i++) {
        struct point *point = paths[i].point;
        while (point != NULL) {
            minx = MIN(minx, point->x);
            miny = MIN(miny, point->y);
            maxx = MAX(maxx, point->x);
            maxy = MAX(maxy, point->y);
            point = point->next;
        }
    }
    printf("minx = %d, miny = %d\n", minx, miny);
    printf("maxx = %d, maxy = %d\n", maxx, maxy);

    int map_size = MAP_WIDTH * (maxy + 2);
    map = malloc(map_size * sizeof(char));
    memset(map, '.', map_size);

    for (int i = 0; i < path_count; i++) {
        struct point *point = paths[i].point;
        while (point->next != NULL) {
            // draw path
            if (point->x == point->next->x) {
                int dir = 1;
                if (point->y > point->next->y) dir = -1;
                for (int y = point->y; y != point->next->y; y += dir) {
                    SET_MAPPOS(point->x, y, '#');
                }
                SET_MAPPOS(point->x, point->next->y, '#');
            } else if (point->y == point->next->y) {
                int dir = 1;
                if (point->x > point->next->x) dir = -1;
                for (int x = point->x; x != point->next->x; x += dir) {
                    SET_MAPPOS(x, point->y, '#');
                }
                SET_MAPPOS(point->next->x, point->y, '#');
            } else {
                printf("illegal diagonal path element.\n");
            }

            point = point->next;
        }
    }
    SET_MAPPOS(500, 0, '+');
}

int simulate_sand(void) {
    int done = 0;
    int sandx = 500, sandy = 1;

    while (! done) {
        if (GET_MAPPOS(sandx, sandy + 1) == '.') {
            sandy += 1;
        } else if (GET_MAPPOS(sandx - 1, sandy + 1) == '.') {
            sandx -= 1;
            sandy += 1; 
        } else if (GET_MAPPOS(sandx + 1, sandy + 1) == '.') {
            sandx += 1;
            sandy += 1;
        } else {
            done = 1;
        }
    }
    SET_MAPPOS(sandx, sandy, 'o');

    return sandy > maxy;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    // for (int i = 0; i < path_count; i++) print_path(i);

    // implement algorithm
    // make a map from the paths
    make_map();
    int done = 0;
    int sand_units = 0;
    while (! done) {
        done = simulate_sand();
        if (! done) sand_units += 1;
    }
    if (debug) print_map();
    printf("There were %d sand units used.\n", sand_units);

    return EXIT_SUCCESS;
}

