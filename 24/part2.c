
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

#define POS_EMPTY 0x00
#define POS_DOWN  0x01
#define POS_LEFT  0x02
#define POS_UP    0x04
#define POS_RIGHT 0x08
#define POS_WALL  0xFF
#define POS_VERTICAL   (POS_DOWN | POS_UP)
#define POS_HORIZONTAL (POS_LEFT | POS_RIGHT)
struct map {
    int time;
    int width;
    int height;
    int *data;
};
struct map map;
struct point {
    int x;
    int y;
} start, end;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    int row_count = 0;
    int col_count = 0;
#define MAX_ROWS 40
    int *maprows[MAX_ROWS];
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (strlen(line) != 0) {
            // parse the data
            int *row = malloc(strlen(line) * sizeof(int));
            for (int i = 0; i < (int)strlen(line); i++) {
                switch (line[i]) {
                case '.':
                    row[i] = POS_EMPTY;
                    break;
                case 'v':
                    row[i] = POS_DOWN;
                    break;
                case '<':
                    row[i] = POS_LEFT;
                    break;
                case '^':
                    row[i] = POS_UP;
                    break;
                case '>':
                    row[i] = POS_RIGHT;
                    break;
                case '#':
                    row[i] = POS_WALL;
                    break;
                default:
                    assert(0 && "Unknown character in map");
                    break;
                }
            }
            maprows[row_count] = row;
            row_count += 1;
            if (col_count == 0) {
                col_count  = strlen(line);
            } else {
                assert(col_count == (int)strlen(line));
            }
        } else {
            fprintf(stderr, "Unexpected input format '%s'.\n", line);
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);

    map.time = 0;
    map.width = col_count;
    map.height = row_count;
    map.data = malloc(map.width * map.height * sizeof(int));
    memset(map.data, 0, map.width * map.height * sizeof(int));
    for (int y = 0; y < map.height; y++) {
        for (int x = 0; x < map.width; x++) {
            int offset = y * map.width + x;
            *(map.data + offset) = maprows[y][x];
        }
        free(maprows[y]);
    }
    start.x = 1; start.y = 0;
    end.x = map.width - 2; end.y = map.height - 1;
}

int getpos(struct map m, int x, int y) {
    int offset = y * m.width + x;
    return *(m.data + offset);
}

void setpos(struct map m, int x, int y, int val) {
    int offset = y * m.width + x;
    *(m.data + offset) = val;
}

int bit_count(int val) {
    if (val == POS_EMPTY) {
        return 0;
    } else if (val == POS_WALL) {
        return 1;
    } else {
        int count = 0;
        for (int i = 0; i < 4; i++) {
            count += (val & 0x01);
            val >>= 1;
        }
        return count;
    }
}

void print_map(struct map m, struct point p) {
    printf("Map after %d minutes:\n", m.time);
    for (int y = 0; y < m.height; y++) {
        for (int x = 0; x < m.width; x++) {
            int val = getpos(m, x, y);
            int bitcount = bit_count(val);
            switch (bitcount) {
            case 0:
                if ((x == p.x) && (y == p.y)) {
                    printf("E");
                } else {
                    printf(".");
                }
                break;
            case 1:
                switch (val) {
                case POS_DOWN:
                    printf("v");
                    break;
                case POS_UP:
                    printf("^");
                    break;
                case POS_RIGHT:
                    printf(">");
                    break;
                case POS_LEFT:
                    printf("<");
                    break;
                case POS_WALL:
                    printf("#");
                    break;
                }
                break;
            default:
                printf("%d", bitcount);
                break;
            }
        }
        printf("\n");
    }
    printf("----------------------\n");
}

unsigned int modulo(int value, unsigned int m) {
    int mod = value % (int)m;
    if (mod < 0) {
        mod += m;
    }
    return mod;
}

// optimization: only calculate the row and column where you are on
struct map make_time_map(int time, struct point pos) {
    assert((pos.x != 0) || (pos.y != 0));
    // allocate the new map
    struct map m;
    m.time = time;
    m.width = map.width;
    m.height = map.height;
    m.data = malloc(m.width * m.height * sizeof(int));
    memset(m.data, 0, m.width * m.height * sizeof(int));

    // perform all left and right movements
    // copy border
    for (int x = 0; x < m.width; x++) setpos(m, x, 0, getpos(map, x, 0));
    for (int x = 0; x < m.width; x++) setpos(m, x, m.height - 1, getpos(map, x, m.height - 1));
    //for (int y = pos.y; y <= pos.y; y++) {
    for (int y = 1; y < m.height - 1; y++) {
        int *row = m.data + y * m.width;
        row[0] = getpos(map, 0, y);
        row[m.width - 1] = getpos(map, m.width - 1, y);
        for (int x = 1; x < m.width - 1; x++) {
            int val = getpos(map, x, y);
            if (val == POS_LEFT) {
                int newx = modulo(x - time - 1, m.width - 2) + 1;
                assert(newx > 0); assert(newx < m.width - 1);
                row[newx] |= POS_LEFT;
            }
            if (val == POS_RIGHT) {
                int newx = modulo(x + time - 1, m.width - 2) + 1;
                assert(newx > 0); assert( newx < m.width - 1);
                row[newx] |= POS_RIGHT;
            }
        }
    }

    // perform all up and down movement
    int *column = malloc(m.height * sizeof(int));
    //for (int x = pos.x; x <= pos.x; x++) {
    for (int x = 1; x < m.width - 1; x++) {
        for (int y = 1; y < m.height - 1; y++) column[y] = getpos(m, x, y);
        for (int y = 1; y < m.height - 1; y++) {
            int val = getpos(map, x, y);
            if (val == POS_UP) {
                int newy = modulo(y - time - 1, m.height - 2) + 1;
                assert(newy > 0); assert(newy < m.height - 1);
                column[newy] |= POS_UP;
            }
            if (val == POS_DOWN) {
                int newy = modulo(y + time - 1, m.height - 2) + 1;
                assert(newy > 0); assert(newy < m.height - 1);
                column[newy] |= POS_DOWN;
            }
        }
        for (int y = 1; y < m.height - 1; y++) setpos(m, x, y, column[y]);
    }
    free(column);
    return m;
}

struct state {
    int time;
    int cost;
    struct point elf;
    struct state *prev;
};

struct state *nextState(struct state *current, struct state *next);
void stateFree(struct state *s);
int stateCompare(const void *v1, const void *v2);
void statePrint(const char *prefix, const void *s);
int costCompare(const void *s1, const void *s2);

#define DIJKSTRA_IMPLEMENTATION
#include "../common/dijkstra.h"
#undef DIJKSTRA_IMPLEMENTATION
#define QUEUE_IMPLEMENTATION
#include "../common/queue.h"
#undef QUEUE_IMPLEMENTATION
#define DICT_IMPLEMENTATION
#include "../common/dict.h"
#undef DICT_IMPLEMENTATION

int stateCompare(const void *v1, const void *v2) {
    state s1 = (state)v1;
    state s2 = (state)v2;
    // same time and same position
    return !((s1->time == s2->time) &&
        (s1->elf.x == s2->elf.x) && (s1->elf.y == s2->elf.y));
}

int costCompare(const void *v1, const void *v2) {
    state s1 = (state)v1;
    state s2 = (state)v2;
    return (s1->time - s2->time);
}

void statePrint(const char *prefix, const void *v) {
    state s = (state)v;
    printf("%s cost = %d, elf = (%d, %d)\n", prefix, s->cost, s->elf.x, s->elf.y);
}

state nextState(state current, state next) {
    assert(current != NULL);
    static int next_index = 0;
    static int state_count = 0;
    static state next_states[6];

    // generate states for wait and 4 directions (when possible)
    if (next == NULL) {
        if (debug) printf("nextState: new states for time = %d.\n", current->time + 1);
        assert((next_states[next_index] == NULL) && "not all generated states were used");
        for (unsigned int i = 0; i < array_count(next_states); i++) next_states[i] = NULL;
        next_index = 0;
        state_count = 0;
        struct map next_map = make_time_map(current->time + 1, current->elf);
        if (debug) { print_map(next_map, current->elf); }
        struct point possibilities[] = {
            { 1, 0},
            { 0, 1},
            { 0, 0},
            { -1, 0},
            { 0, -1},
        };
        for (unsigned int d = 0; d < array_count(possibilities); d++) {
            int x = current->elf.x + possibilities[d].x;
            int y = current->elf.y + possibilities[d].y;
            if ((y >= 0) && (y <= next_map.height - 1)) {
                int val = getpos(next_map, x, y);
                if (val == POS_EMPTY) {
                    state s = malloc(sizeof(struct state));
                    s->time = current->time + 1;
                    s->cost = current->cost + 1;
                    s->elf.x = x; s->elf.y = y;
                    s->prev = NULL;
                    if (debug) statePrint("genstate:", s);
                    next_states[state_count] = s;
                    state_count += 1;
                }
            }
        }
        if (debug) printf("genstates: %d created.\n", state_count);
        free(next_map.data);
    }

    if (debug) printf("nextState: returning index %d of %d.\n", next_index, state_count);
    if (debug) printf("nextState: returning %p.\n", (void *)next_states[next_index]);
    return next_states[next_index++];
}

void stateFree(state s) {
    if (s != NULL) {
        free(s);
    }
}

int isEndState1(state s) {
    return ((s->elf.x == end.x) && (s->elf.y == end.y));
}

int isEndState2(state s) {
    return ((s->elf.x == start.x) && (s->elf.y == start.y));
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    struct point pos = start;
    printf("Map is %dx%d.\n", map.width, map.height);
    //debug = 1;
    if (debug) print_map(map, pos);

    // implement algorithm
    struct state start_state;
    start_state.time = 0;
    start_state.cost = 0;
    start_state.elf = start;
    start_state.prev = NULL;
    struct state end_state;
    end_state.time = INT_MAX;
    end_state.cost = INT_MAX;
    end_state.elf = end;
    end_state.prev = NULL;
    struct state end_state2;
    end_state2.time = INT_MAX;
    end_state2.cost = INT_MAX;
    end_state2.elf = start;
    end_state2.prev = NULL;
    struct state end_state3;
    end_state3.time = INT_MAX;
    end_state3.cost = INT_MAX;
    end_state3.elf = end;
    end_state3.prev = NULL;

    //debug = 1;
    state e = dijkstra(&start_state, &end_state, isEndState1);
    statePrint("Final state1:", e);
    start_state.time = e->cost;
    start_state.cost = e->cost;
    start_state.elf = e->elf;
    e = dijkstra(&start_state, &end_state2, isEndState2);
    statePrint("Final state2:", e);
    start_state.time = e->cost;
    start_state.cost = e->cost;
    start_state.elf = e->elf;
    e = dijkstra(&start_state, &end_state3, isEndState1);
    statePrint("Final state3:", e);

    printf("Reverse path:\n");
    state current = e;
    while (current != NULL) {
        statePrint("  ", current);
        current = current-> prev;
    }
#if 0
    int time = 0;
    int reached_exit = 0;
    while (! reached_exit) {
        time += 1;
        struct map m = make_time_map(time, start);

        if (debug) print_map(m, pos);
        free(m.data);

        if (time == 4) break;
        if (time == 18) break;
    }
#endif
    return EXIT_SUCCESS;
}

