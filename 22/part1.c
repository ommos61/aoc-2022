
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 10240
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output

int row_count = 0;
int max_column= 0;
#define MAX_ROWS 200
char *maprows[MAX_ROWS];
char *directions = NULL;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    int reading_map = 1;
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (reading_map && (strlen(line) != 0)) {
            // store the map data
            char *row = malloc(strlen(line) + 1);
            strcpy(row, line);
            maprows[row_count] = row;
            row_count += 1;
            max_column = MAX((unsigned)max_column, strlen(line) + 1);
        } else if (reading_map && (strlen(line) == 0)) {
            reading_map = 0;
        } else if (! reading_map) {
            directions = malloc(strlen(line) + 1);
            strcpy(directions, line);
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

#define WALL '#'
#define OPEN '.'
struct point {
    int x;
    int y;
};
struct point dir_sequence[] = {
    { 1, 0 },
    { 0, 1 },
    { -1, 0 },
    { 0, -1 },
};
char *dir_chars = ">v<^";

struct point find_start(void) {
    struct point start = { 0, 0 };
    for (unsigned int i = 0; i < strlen(maprows[0]); i++) {
        if (maprows[0][i] == OPEN) {
            start.x = i;
            break;
        }
    }

    return start;
}

#define MOVE_NONE  0
#define MOVE_LEFT  1
#define MOVE_RIGHT 2
#define MOVE_COUNT 3
struct move {
    int type;
    int count;
};
int move_index= 0;

char getpos(struct point pos) {
    return (pos.x >= (int)strlen(maprows[pos.y])) ? ' ' : maprows[pos.y][pos.x];
}

void setpos(struct point pos, char c) {
    maprows[pos.y][pos.x] = c;
}

void print_map(void) {
    for (int y = 0; y < row_count; y++) {
        printf("%s\n", maprows[y]);
    }
    printf("-------------------\n");
}

void print_move(struct move move) {
    switch (move.type) {
        case  MOVE_LEFT:
            printf("Move: turn left.\n");
            break;
        case MOVE_RIGHT:
            printf("Move: turn right.\n");
            break;
        case MOVE_COUNT:
            printf("Move: %d forward.\n", move.count);
            break;
        default:
            printf("Move: unknown.\n");
            break;
        }
}

struct move next_move(void) {
    struct move move;
    move.type = MOVE_NONE;

    int value = 0;
    if (directions[move_index] == 'L') {
        move.type = MOVE_LEFT;
        move_index += 1;
    } else if (directions[move_index] == 'R') {
        move.type = MOVE_RIGHT;
        move_index += 1;
    } else if (sscanf(directions + move_index, "%d", &value) == 1) {
        move.type = MOVE_COUNT;
        move.count = value;
        while (isdigit(directions[move_index]))  {
            move_index  +=1;
        }
    } else if (directions[move_index] == '\000') {
        // fall through with MOVE_NONE
    } else {
        assert(0 && "Unexpected input format");
    }

    return move;
}

struct point moveto_direction(struct point pos, int dir) {
    struct point newpos = pos;
    //printf("newpos1 = (%d, %d)\n", newpos.x, newpos.y);
    newpos.x += dir_sequence[dir].x;
    newpos.y += dir_sequence[dir].y;
    //printf("newpos2 = (%d, %d)\n", newpos.x, newpos.y);
    // check on bounds
    if (newpos.y < 0) {
        newpos.y = row_count - 1;
    }
    if (newpos.y >= row_count) {
        newpos.y = 0;
    }
    if (newpos.x < 0) {
        newpos.x = strlen(maprows[newpos.y]) - 1;
    }
    if (newpos.x >= max_column) {
        newpos.x = 0;
    }
    //printf("newpos3 = (%d, %d), max_column = %d\n", newpos.x, newpos.y, max_column);
    return newpos;
}

struct point move_one(struct point pos, int dir) {
    struct point newpos = moveto_direction(pos, dir);

    // check on space or wall
    char c = getpos(newpos);
    while (c == ' ') {
        newpos = moveto_direction(newpos, dir);
        c = getpos(newpos);
    }
    if (c == '#') {
        newpos = pos;
    } 
    setpos(newpos, dir_chars[dir]);
    return newpos;
}

void perform_move(struct move move, struct point *pos, int *dir)  {
    if (move.type == MOVE_RIGHT) {
        *dir = (*dir + 1) % 4;
        setpos(*pos, dir_chars[*dir]);
    } else if (move.type== MOVE_LEFT) {
        *dir = (*dir + 4 - 1) % 4;
        setpos(*pos, dir_chars[*dir]);
    } else if (move.type == MOVE_COUNT) {
        struct point newpos = *pos;
        assert(move.count > 0);
        while (move.count > 0) {
            newpos = move_one(newpos, *dir);
            move.count -= 1;
        }
        *pos = newpos;
    }
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("Map has %d rows.\n", row_count);
    printf("Map has %d columns.\n", max_column);
    printf("Directions is %lu characters long.\n", strlen(directions));

    // implement algorithm
    struct point pos = find_start();
    int dir = 0;
    printf("Start is at (%d, %d), facing '%c'.\n", pos.x, pos.y, dir_chars[dir]);
    maprows[pos.y][pos.x] = dir_chars[dir];

    struct move move = next_move();
    while (move.type != MOVE_NONE) {
        //printf("(%d, %d) ", pos.x, pos.y);
        //print_move(move);
        if (debug) print_move(move);
        perform_move(move, &pos, &dir);
        //printf("(%d, %d) ", pos.x, pos.y);
        //printf("Facing = %d '%c'.\n", dir, dir_chars[dir]);
        if (debug) print_map();

        move = next_move();
    }
    printf("Position is (%d, %d)\n", pos.x + 1, pos.y + 1);
    printf("Password is %d.\n", 1000 * (pos.y + 1) + 4 * (pos.x + 1) + dir);

    printf("The answer 35450 is too low.\n");
    return EXIT_SUCCESS;
}

