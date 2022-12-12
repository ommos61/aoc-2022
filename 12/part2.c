
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output

int map_rows = 0, map_cols = 0;
#define MAX_ROWS 50
#define MAX_COLS 200
int map[MAX_ROWS][MAX_COLS];
struct point {
    int x; int y;
};
int lowpoint_count = 0;
#define MAX_LOWPOINTS (MAX_ROWS * MAX_COLS)
struct point lowpoints[MAX_LOWPOINTS];

int startx = -1, starty = -1;
int endx = -1, endy = -1;

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
            map_cols = strlen(line);
            for (int x = 0; x < map_cols; x++) {
                if (line[x] == 'S') {
                    startx = x; starty = map_rows;
                    map[map_rows][x] = 'a' - 'a';
                    lowpoints[lowpoint_count].x = x;
                    lowpoints[lowpoint_count].y = map_rows;
                    lowpoint_count++;
                } else if (line[x] == 'E') {
                    endx = x; endy = map_rows;
                    map[map_rows][x] = 'z' - 'a';
                } else {
                    map[map_rows][x] = line[x] - 'a';
                    if (line[x] == 'a') {
                        lowpoints[lowpoint_count].x = x;
                        lowpoints[lowpoint_count].y = map_rows;
                        lowpoint_count++;
                    }
                }
            }
            map_rows += 1;
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

void print_map(void) {
    for (int y = 0; y < map_rows; y++) {
        for (int x = 0; x < map_cols; x++) {
            printf("%c", 'a' + map[y][x]);
        }
        printf("\n");
    }
}

char *visited = NULL;
int *steps = NULL;
#define HEIGHT(x, y)         (map[(y)][(x)])
#define VISITED(x, y)        (*(visited + (map_cols * (y)) + (x)))
#define SET_VISITED(x, y, d) (*(visited + (map_cols * (y)) + (x)) = (d))
#define STEPS(x, y)          (*(steps + (map_cols * (y)) + (x)))
#define SET_STEPS(x, y, s)   (*(steps + (map_cols * (y)) + (x)) = (s))

struct point directions[5] = {
    {  0,  0 },
    {  0, -1 },
    {  0,  1 },
    { -1,  0 },
    {  1,  0 }
};
char *direction_chars = ".^v<>";

#define IS_VALID_POS(x, y) (((x) >= 0) && ((x) < map_cols) && ((y) >= 0) && ((y) < map_rows))

void print_visited(void) {
    for (int y = 0; y < map_rows; y++) {
        for (int x = 0; x < map_cols; x++) {
            printf("%c", VISITED(x, y));
        }
        printf("\n");
    }
}

void do_moves(struct point);

void do_move(struct point pos, struct point dir) {
    struct point dest = { pos.x + dir.x, pos.y + dir.y };

    // check if out of bounds
    if (! IS_VALID_POS(dest.x, dest.y)) return;

    // Check if the height allows the move
    int pos_height = HEIGHT(pos.x, pos.y);
    int dest_height = HEIGHT(dest.x, dest.y);
    if (dest_height > pos_height + 1) return;

    int pos_steps = STEPS(pos.x, pos.y);
    int dest_steps = STEPS(dest.x, dest.y);
    if ((VISITED(dest.x, dest.y) == '.') || (pos_steps + 1 < dest_steps)) {
        // Update the steps
        SET_STEPS(dest.x, dest.y, pos_steps + 1);
        //
        // Mark as dest as VISITED
        SET_VISITED(dest.x, dest.y, '#');

        // and recurse if not the end reached
        if ((dest.x == endx) && (dest.y == endy)) {
            if (debug) printf("Destination reached in %d steps.\n", STEPS(endx, endy));
        } else {
            do_moves(dest);
        }
    }
}

void do_moves(struct point pos) {
    for (int dir = 1; dir <= 4; dir++) {
        do_move(pos, directions[dir]);
    }
}

int find_path(struct point start) {
    int map_size = map_rows * map_cols;
    visited = malloc(map_size * sizeof(char));
    memset(visited, '.', map_size);
    steps = malloc(map_size * sizeof(int));
    memset(steps, 0, map_size * sizeof(int));

    struct point pos = { start.x, start.y };
    // Mark as start as VISITED
    SET_VISITED(pos.x, pos.y, '#');
    do_moves(pos);

    return STEPS(endx, endy);
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    assert((startx != -1) && (starty != -1));
    assert((endx != -1) && (endy != -1));
    printf("Map is %dx%d\n", map_cols, map_rows);
    printf("There are %d lowpoints.\n", lowpoint_count);
    if (debug) print_map();

    // implement algorithm
    int min_steps = map_rows * map_cols;
    for (int i = 0; i < lowpoint_count; i++) {
        int steps = find_path(lowpoints[i]);
        if (steps != 0) {
            if (debug) printf("Steps to the end is %d\n", steps);
            min_steps = MIN(min_steps, steps);
        }
    }
    printf("Minimum steps count is %d.\n", min_steps);

    return EXIT_SUCCESS;
}

