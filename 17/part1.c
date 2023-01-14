
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

int jetpattern_size = 0;
#define MAX_JETPATTERN 20000
char jetpattern[MAX_JETPATTERN];

char *rock1[4] = { "....", "....", "....", "@@@@" };
char *rock2[4] = { "...", ".@.", "@@@", ".@." };
char *rock3[4] = { "...", "..@", "..@", "@@@" };
char *rock4[4] = { "@", "@", "@", "@" };
char *rock5[4] = { "..", "..", "@@", "@@" };

char **rocks[] =     { rock1, rock2, rock3, rock4, rock5 };
int rock_heights[] = { 1,     3,     3,     4,     2 };
int rock_widths[] =  { 4,     3,     3,     1,     2 };

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[MAX_JETPATTERN];
    while (fgets(line, MAX_JETPATTERN, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (strlen(line) != 0) {
            // parse the data
            strcpy(jetpattern, line);
            jetpattern_size = strlen(line);
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

char field[10000][7];
char rock[4][7];
int rock_y = 0; // where the bottom of the rock is
int rock_x = 0;
int next_shape = 0;
int next_jet = 0;
int rock_falling = 0;
int rock_height = 0;
int rock_width = 0;

void print_rock(void) {
    printf("Rock:\n");
    for (int y = 3; y >= 0; y--) {
        printf("|");
        for (int x = 0; x < rock_width; x++) {
            printf("%c", rock[y][x]);
        }
        printf("|\n");
    }
}

void print_field(int height) {
    printf("Field:\n");
    for (int h = height + 1; h >= 0; h--) {
        printf("|");
        for (int x = 0; x < 7; x++) {
            char c = field[h][x];
            if ((h >= rock_y) && (h < rock_y + rock_height) &&
                (x >= rock_x) && (x < rock_x + rock_width)) {
                c = rock[h - rock_y][x - rock_x];
            }
            printf("%c", c);
        }
        printf("|\n");
    }
    printf("+-------+\n\n");
}

char get_direction(void) {
    char dir = jetpattern[next_jet];
    next_jet = (next_jet + 1) % jetpattern_size;

    return dir;
}

int rock_overlap(int new_rock_y, int new_rock_x) {
    int overlapping = 0;

    if (new_rock_y < 0) {
        overlapping = 1;
    } else {
        for (int y = 0; y < rock_height; y++) {
            for (int x = 0; x < rock_width; x++) {
                if ((rock[y][x] != '.') && (field[new_rock_y + y][new_rock_x + x] != '.')) {
                    overlapping = 1;
                    break;
                }
            }
            if (overlapping) break;
        }
    }

    return overlapping;
}

void rock_push(void) {
    char dir = get_direction();
    if (debug) printf("Push: %c\n", dir);
    switch (dir) {
    case '<':
        if ((rock_x > 0) && ! rock_overlap(rock_y, rock_x - 1)) {
            rock_x -= 1;
        }
        break;
    case '>':
        if ((rock_x + rock_width < 7) && ! rock_overlap(rock_y, rock_x + 1)) {
            rock_x += 1;
        }
        break;
    default:
        assert(0 && "Unknown direction for jet stream");
        break;
    }
}

int rock_fall(void) {
    int still_falling = 1;

    if (rock_overlap(rock_y - 1, rock_x)) {
        still_falling = 0;
    } else {
        rock_y -= 1;
    }

    return still_falling;
}

int simulate_rock(int height) {
    assert(height >= 0);

    if (! rock_falling) {
        // get new rock
        rock_falling = 1;
        rock_height = rock_heights[next_shape];
        rock_width = rock_widths[next_shape];
        for (int y = 0; y < 4; y++) {
            char *rock_line = rocks[next_shape][3 - y];
            for (int x = 0; x < rock_width; x++) {
                rock[y][x] = rock_line[x];
            }
        }
        rock_y = height + 3;
        rock_x = 2;

        next_shape = (next_shape + 1) % array_count(rocks);
    }
    int print_height = rock_y + rock_height;
    if (debug) print_field(print_height);
    while (rock_falling) {
        // push the rock by a jet stream
        rock_push();
        // let the rock fall one unit
        rock_falling = rock_fall();
        if (! rock_falling) {
            // Fix the rock in the field
            for (int y = 0; y < rock_height; y++) {
                for (int x = 0; x < rock_width; x++) {
                    char rock_piece = rock[y][x];
                    if (rock_piece != '.') {
                        field[rock_y + y][rock_x + x] = '#';
                        rock[y][x] = '#';
                    }
                }
            }
            height = MAX(height, rock_y + rock_height);
        }

        //print_field(print_height);
    }

    return height;
}

int simulate(int count) {
    assert(count > 0);
    int current_height = 0;
    memset(field, '.', sizeof(field));
    memset(rock, 0, sizeof(rock));
    next_shape = 0;
    next_jet = 0;
    rock_falling = 0;

    while (count > 0) {
        current_height = simulate_rock(current_height);
        //print_field(current_height + 3 + rock_height);
        count -= 1;
    }
    print_field(current_height + 3 + rock_height);

    return current_height;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %d items in the jet pattern.\n", jetpattern_size);

    // implement algorithm
    int rock_count = 2022;
    //rock_count = 20;
    int height = simulate(rock_count);
    printf("The height after %d rocks is %d.\n", rock_count, height);

    return EXIT_SUCCESS;
}

