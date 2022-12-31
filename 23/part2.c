
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output

int row_count = 0;
int col_count = 0;
#define MAX_ROWS 200
#define MAX_COLS 200
char *maprows[MAX_ROWS];

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

        if (strlen(line) > 0) {
            // store the row
            char *row = malloc(MAX_COLS + 1);
            strcpy(row, line);
            maprows[row_count] = row;
            row_count += 1;
            if (col_count == 0) {
                col_count = strlen(row);
            } else {
                assert((col_count == (int)strlen(row)) && "rows must have same width");
            }
        } else {
            fprintf(stderr, "Unexpected input format '%s'.\n", line);
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

void print_map(void) {
    for (int y = 0; y < row_count; y++) {
        printf("%s\n", maprows[y]);
    }
    printf("------------------\n");
}

char getpos(int x, int y) {
    char c = '.';
    if ((x >= 0) && (y >= 0) && (x < col_count) && (y < row_count)) {
        c = maprows[y][x];
    }
    return c;
}

void setpos(int x, int y, char val) {
    maprows[y][x] = val;
}

char *dir_chars = "NSWE";
struct point {
    int x;
    int y;
} dirs[] = {
    { 0, -1 }, // N
    { 0, 1 },  // S
    { -1, 0 }, // W
    { 1, 0 },  // E
};

int next_dir(int dir_index) {
    int new_dir = (dir_index + 1) % 4;
    return new_dir;
}

struct point all_dirs[] = {
    { -1, -1 }, // NW
    {  0, -1 }, // N
    {  1, -1 },
    {  1,  0 }, // E
    {  1,  1 },
    {  0,  1 }, // S
    { -1,  1 },
    { -1,  0 }, // W
    { -1, -1 }, // NW
};
int all_offsets[] = {
    0, // N
    4, // S
    6, // W
    2, // E
};

int check_occupied(int x, int y, char *occupied) {
    int count = 0;
    for (int i = 0; i < 9; i++) {
        if (getpos(x + all_dirs[i].x, y + all_dirs[i].y) != '.') {
            occupied[i] = 1;
            count += 1;
        } else {
            occupied[i] = 0;
        }
    }
    return count;
}

void round_propose(int first_dir) {
    assert((first_dir >= 0) && (first_dir < 4));
    for (int y = 0; y < row_count; y++) {
        for (int x = 0; x < col_count; x++) {
            if (getpos(x, y) == '#') {
                char occupied[9];
                int count = check_occupied(x, y, occupied);
                // check if no positions occupied -> no move
                if (count == 0) {
                    setpos(x, y, '=');
                    continue;
                }
                // loop over the direction sides -> pick the first empty
                int dir = first_dir;
                do {
                    int side_count = 0;
                    int start_index = all_offsets[dir];
                    for (int i = start_index; i < start_index + 3; i++) {
                        if (occupied[i]) {
                            side_count += 1;
                        }
                    }
                    if (side_count == 0) {
                        setpos(x, y, dir_chars[dir]);
                        break;
                    }
                    dir = next_dir(dir);
                } while (dir != first_dir);
                if (getpos(x, y) == '#') setpos(x, y, '=');
            }
        }
    }
}

void resize_map(void) {
    // first check for resize to W and E
    int west = 0, east = 0;
    for (int y = 0; y < row_count; y++) {
        if (getpos(0, y) == 'W') west = 1;
        if (getpos(col_count - 1, y) == 'E') east = 1;
    }
    if (west + east > 0) {
        assert(col_count + west + east < MAX_COLS - 1);
        char temp[MAX_COLS];
        for (int y = 0; y < row_count; y++) {
            *temp = 0;
            if (west) strcpy(temp, ".");
            strcat(temp, maprows[y]);
            if (east) strcat(temp, ".");
            strcpy(maprows[y], temp);
        }
        col_count += west + east;
    }
    // next check for resize to N and S
    int north = 0, south = 0;
    for (int x = 0; x < col_count; x++) {
        if (getpos(x, 0) == 'N') north = 1;
        if (getpos(x, row_count - 1) == 'S') south = 1;
    }
    assert(row_count + north + south < MAX_ROWS);
    if (north) {
        memmove(maprows + 1, maprows, row_count * sizeof(char *));
        maprows[0] = malloc(MAX_COLS + 1);
        for (int x = 0; x < col_count; x++) maprows[0][x] = '.';
        maprows[0][col_count] = 0;
        row_count += north;
    }
    if (south) {
        maprows[row_count] = malloc(MAX_COLS + 1);
        for (int x = 0; x < col_count; x++) maprows[row_count][x] = '.';
        maprows[row_count][col_count] = 0;
        row_count += south;
    }
}

int round_move(void) {
    int move_count = 0;
    for (int y = 0; y < row_count; y++) {
        for (int x = 0; x < col_count; x++) {
            char c = getpos(x, y);
            if (c == '=') {
                setpos(x, y, '#');
            } else if (c == '.') {
                // see what wants to move here
                int sources[4];
                int source_count = 0;
                char *moves = "SNEW";
                for (int d = 0; d < 4; d++) {
                    char csource = getpos(x + dirs[d].x, y + dirs[d].y);
                    if (csource == moves[d]) {
                        sources[source_count] = d;
                        source_count += 1;
                    }
                }
                if (source_count == 1) {
                    setpos(x + dirs[sources[0]].x, y + dirs[sources[0]].y, '.');
                    setpos(x, y, '#');
                    move_count += 1;
                } else {
                    for (int i = 0; i < source_count; i++) {
                        setpos(x + dirs[sources[i]].x, y + dirs[sources[i]].y, '#');
                    }
                }
            }
        }
    }
    return move_count;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("Map is %dx%d.\n", col_count, row_count);
    if (debug) print_map();

    // implement algorithm
    int dir_index = 0;
    int move_count = 0;
    int round_count = 0;
    do {
        round_propose(dir_index);
        if (debug) print_map();
        resize_map();
        move_count = round_move();
        dir_index = next_dir(dir_index);
        round_count += 1;

        if (debug) printf("Map after round %d, first_dir = %d '%c'.\n", round_count, dir_index, dir_chars[dir_index]);
        if (debug) print_map();
    } while (move_count != 0);
    printf("Number of rounds until the elves stop moving is %d.\n", round_count);

    return EXIT_SUCCESS;
}

