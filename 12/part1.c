
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
                } else if (line[x] == 'E') {
                    endx = x; endy = map_rows;
                    map[map_rows][x] = 'z' - 'a';
                } else {
                    map[map_rows][x] = line[x] - 'a';
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

struct point {
    int x; int y;
};
int move_count = 0;
#define MAX_MOVES 5000
struct point moves[MAX_MOVES];
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

struct point get_next_point(struct point pos) {
    struct point next = pos;

    int dir_index = 0;
    int done = 0;
    while (! done) {
        // find the possible next direction in the sequence
        while (VISITED(pos.x, pos.y) != direction_chars[dir_index]) {
            dir_index += 1;
        }
        dir_index += 1;
        //printf("pos = (%d, %d), dir_index = %d, next_dir = %c\n", pos.x, pos.y, dir_index, direction_chars[dir_index]);
        
        if ((unsigned)dir_index < strlen(direction_chars)) {
            SET_VISITED(pos.x, pos.y, direction_chars[dir_index]);
            next.x = pos.x + directions[dir_index].x;
            next.y = pos.y + directions[dir_index].y;
            if (IS_VALID_POS(next.x, next.y)) {
                done = 1;
            } else {
                next = pos;
            }
        } else {
            break;
        }
    }

    return next;
}

int find_path(int max_loops) {
    int loop_count = 0;
    int map_size = map_rows * map_cols;
    visited = malloc(map_size * sizeof(char));
    char *shortest = malloc(map_size * sizeof(char));
    steps = malloc(map_size * sizeof(int));

    memset(visited, '.', map_size);
    memset(steps, 0, map_size * sizeof(int));
    struct point pos = { startx, starty };
    int min_move_count = map_rows * map_cols;
    while (! ((pos.x == endx) && (pos.y == endy))) {
        int current_height = HEIGHT(pos.x, pos.y);
        //printf("current height = %d\n", HEIGHT(pos.x, pos.y));
        moves[move_count] = pos;
        assert(move_count < MAX_MOVES);
        struct point next = get_next_point(pos);
        
        if ((next.x == pos.x) && (next.y == pos.y)) {
            // backtrack
            if (debug) printf("Backtracking...\n");
            SET_VISITED(pos.x, pos.y, '.');
            move_count -= 1;
            if (move_count < 0) {
                if (debug) printf("Search paths exhausted.\n");
                break;
            }
            assert(move_count >= 0);
            pos = moves[move_count];
        } else {
            if (debug) printf("next = (%d, %d)\n", next.x, next.y);
            if (debug) printf("next height = %d\n", HEIGHT(next.x, next.y));
            if ((VISITED(next.x, next.y) == '.') &&
                (HEIGHT(next.x, next.y) <= current_height + 1)) {
                //printf("moving to next\n");
                pos.x = next.x; pos.y = next.y;
                move_count++;
            } else {
                //printf("next was not valid\n");
            }
        }

        // Check is reached goal and update minimum
        if ((pos.x == endx) && (pos.y == endy)) {
            printf("Ideal position reached in %d moves!!!\n", move_count);
            if (move_count < min_move_count) {
                min_move_count = move_count;
                if (debug) printf("Updated minimal move count to %d\n", min_move_count);
                SET_VISITED(endx, endy, 'E');
                if (debug) print_visited();
                memcpy(shortest, visited, map_size);
                SET_VISITED(endx, endy, '.');
            }
            // TODO: save the route
            // backtrack
            if (debug) printf("Backtracking for shorter route...\n");
            SET_VISITED(pos.x, pos.y, '.');
            move_count -= 1;
            if (move_count < 0) {
                if (debug) printf("Search paths exhausted.\n");
                break;
            }
            assert(move_count >= 0);
            pos = moves[move_count];
        }

        // failsafe to exit loop
        loop_count += 0;
        if (loop_count > max_loops) {
            printf("Loop failsafe reached, braking....\n");
            break;
        }
    }

    printf("Shortest path was %d:\n", min_move_count);
    memcpy(visited, shortest, map_size);
    if (debug) {
        print_map();
        printf("-----------------------\n");
        print_visited();
    }
    return min_move_count;
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

int find_path2(int dummy) {
    assert(dummy >= 0);
    int map_size = map_rows * map_cols;
    visited = malloc(map_size * sizeof(char));
    memset(visited, '.', map_size);
    steps = malloc(map_size * sizeof(int));
    memset(steps, 0, map_size * sizeof(int));

    struct point pos = { startx, starty };
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
    if (debug) print_map();

    // implement algorithm
#define MAX_LOOPS 1000000000
    int steps = find_path2(MAX_LOOPS);
    printf("Steps to the end is %d\n", steps);
    printf("474 steps was too high\n");

    return EXIT_SUCCESS;
}

