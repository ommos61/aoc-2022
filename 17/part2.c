
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

char rock1[4] = { 0x00, 0x00, 0x00, 0x78 };
char rock2[4] = { 0x00, 0x20, 0x70, 0x20 };
char rock3[4] = { 0x00, 0x10, 0x10, 0x70 };
char rock4[4] = { 0x40, 0x40, 0x40, 0x40 };
char rock5[4] = { 0x00, 0x00, 0x60, 0x60 };

char *rocks[] =     { rock1, rock2, rock3, rock4, rock5 };
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

#define MAX_FIELD 1000000
char field[MAX_FIELD];
#define TETRIS 0x7F
long field_offset_y = 0;
char rock[4];
int rock_y = 0; // where the bottom of the rock is
int rock_x = 0;
int next_shape = 0;
int next_jet = 0;
int rock_height = 0;
int rock_width = 0;

void print_rock(void) {
    printf("Rock:\n");
    for (int y = rock_height - 1; y >= 0; y--) {
        printf("|");
        for (int j = 0; j < rock_width; j++) {
            if ((rock[y] & (0x40 >> j)) == 0) {
                printf(".");
            } else {
                printf("@");
            }
        }
        printf("|\n");
    }
}

void print_field(long height, int depth) {
    printf("Top of field:\n");
    for (int h = height; (h >= 0) && (h >= height - depth); h--) {
        printf("|");
        for (int x = 6; x >= 0; x--) {
            char c = ((field[h] & (1 << x)) == 0) ? '.' : '#';
            if ((h >= rock_y) && (h < rock_y + rock_height)) {
                c = (((rock[h - rock_y] >> rock_x) & (1 << x)) != 0) ? '@' : c;
            }
            printf("%c", c);
        }
        printf("|\n");
    }
    printf("+-------+\n");
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
            if ((field[new_rock_y + y] & (rock[y] >> new_rock_x)) != 0x00) {
                overlapping = 1;
                break;
            }
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

long simulate_rock(long height) {
    assert(height >= 0);
    int rock_falling = 0;

    if (! rock_falling) {
        // get new rock
        rock_falling = 1;
        rock_height = rock_heights[next_shape];
        rock_width = rock_widths[next_shape];
        for (int y = 0; y < 4; y++) {
            char rock_line = rocks[next_shape][3 - y];
            rock[y] = rock_line;
        }
        rock_y = height + 3;
        rock_x = 2;
        //print_rock();

        next_shape = (next_shape + 1) % array_count(rocks);
    }

    //long print_height = rock_y + rock_height;
    //if (debug) print_field(print_height);
    while (rock_falling) {
        // push the rock by a jet stream
        rock_push();
        if (debug) print_field(height + 10, 10);
        // let the rock fall one unit
        rock_falling = rock_fall();
        if (debug) printf("Drop 1\n");
        if (debug) print_field(height + 10, 10);
        if (! rock_falling) {
            // Fix the rock in the field
            for (int y = 0; y < rock_height; y++) {
                field[rock_y + y] |= (rock[y] >> rock_x);
            }
            height = MAX(height, rock_y + rock_height);
            // clear the rock, because it is frozen now
            memset(rock, 0x00, sizeof(rock));

            //if (debug) print_field(print_height);
#if 0
            // check if a TETRIS is created and the field can be reduced
            for (long y = rock_y + rock_height - 1; y >= rock_y; y--) {
                if (field[y] == 0x7F) {
                    if (debug) printf("Encountered TETRIS in line %ld.\n", field_offset_y + y);
                    memmove(field, field + y + 1, height - y);
                    height -= y + 1;
                    field_offset_y += y + 1;

                    // clear the remainder of the field
                    memset(field + height + 1, 0x00, MAX_FIELD - height - 2);
                    if (debug) print_field(height + 1, 10);
                    //debug = 1;
                    break;
                }
            }
#endif
        }

        //print_field(print_height);
    }

    return height;
}

struct state {
    int shape_index;
    int jet_index;
    int depths[7];

    long height;
    long rocks;
    int hash;
    struct state *next;
};
#define MAX_STATES 5000
struct state *seen[MAX_STATES] = { NULL };

int hash_state(int shape, int jet, int *depths) {
    int hash = shape ^ jet;
    for (int i = 0; i < 7; i++) hash ^= depths[i];
    return hash % MAX_STATES;
}

void print_state(struct state *s) {
    printf("shape: %d, jet: %d, { ", s->shape_index, s->jet_index);
    for (int i = 0; i < 7; i++) printf("%d, ", s->depths[i]);
    printf("} = height: %ld, rock: %ld\n", s->height, s->rocks);
}

void clear_states(void) {
    for (int i = 0; i < MAX_STATES; i++) {
        struct state *s = seen[i];
        while (s != NULL) {
            struct state *n = s->next;
            free(s);
            s = n;
        }
        seen[i] = NULL;
    }
}

void new_state(int shape, int jet, int *depths, long height, long rocks) {
    struct state *new = malloc(sizeof(struct state));
    new->shape_index = shape;
    new->jet_index = jet;
    for (int i = 0; i < 7; i++) new->depths[i] = depths[i];
    new->height = height;
    new->rocks = rocks;
    new->hash = hash_state(shape, jet, depths);
    new->next = seen[new->hash];
    seen[new->hash] = new;
}

struct state *find_state(int shape, int jet, int *depths) {
    struct state *states = seen[hash_state(shape, jet, depths)];
    while (states != NULL) {
        int found = (states->shape_index == shape) && (states->jet_index == jet);
        for (int i = 0; i < 7; i++) {
            if (states->depths[i] != depths[i]) {
                found = 0;
                break;
            }
        }
        if (found) break;
        states = states->next;
    }
    return states;
}

void determine_depths(int *depths, int height) {
    // TODO determine actual depths
    for (int i = 0; i < 7; i++) {
        int depth = 0;

        while ((height - depth > 0) && 
               ((field[height - depth] & (0x40 >> i)) == 0)) {
            depth += 1;
        }
        depths[i] = depth;
    }
}

long simulate(long count) {
    long height = 0;
    memset(field, 0x00, sizeof(field));
    memset(rock, 0x00, sizeof(rock));
    next_shape = 0;
    next_jet = 0;

    long rock = 0;
    long offset = 0;
    while (rock < count) {
        height = simulate_rock(height);
        int shape = next_shape;
        int jet = next_jet;
        int depths[7];
        determine_depths(depths, height);
        struct state *s = find_state(shape, jet, depths);
        if (s != NULL) {
            printf("Repeat found at %ld.\n", height);
            printf("shape: %d, jet: %d, { ", shape, jet);
            for (int i = 0; i < 7; i++) printf("%d, ", depths[i]);
            printf("} -> height: %ld, rock: %ld\n", height, rock);
            print_state(s);
            long repeat = (count - rock) / (rock - s->rocks);
            offset = repeat * (height - s->height);
            rock += repeat * (rock - s->rocks);
            //return height + offset;
            clear_states();
        }
        new_state(shape, jet, depths, height, rock);
        //print_field(current_height + 3 + rock_height);
        rock += 1;
    }
    if (debug) print_field(height + 3 + rock_height, 10);

    return height + offset;
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
    long rock_count = 2022;
    //rock_count = 20;
    rock_count = 1000000000000;
    printf("Simulating %ld falling rocks.\n", rock_count);
    long height = simulate(rock_count);
    printf("The height after %ld rocks is %ld.\n", rock_count, height);
    if (strcmp(fname, "input.txt") != 0) {
        printf("Test data reaches height of %ld.\n", 1514285714288);
    }

    return EXIT_SUCCESS;
}

