
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
            max_column = MAX(max_column, (int)strlen(line));
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

void fillout_rows(void) {
    for (int y = 0; y < row_count; y++) {
        if (max_column != (int)strlen(maprows[y])) {
            char *newrow = malloc(max_column + 1);
            strcpy(newrow, maprows[y]);
            while (strlen(newrow) < (unsigned)max_column) {
                strcat(newrow, " ");
            }
            free(maprows[y]);
            maprows[y] = newrow;
        }
    }
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

int tile_size = 0;
void print_map(struct point pos, int size) {
    int startx = 0, starty = 0;
    int endx = max_column, endy = row_count;
    if (tile_size > size) {
        startx = MAX(0, pos.x - size / 2);
        endx = MIN(startx + size, max_column);
        starty = MAX(0, pos.y - size / 2);
        endy = MIN(starty + size, row_count);
    }
    for (int y = starty; y < endy; y++) {
        for (int x = startx; x < endx; x++) {
            printf("%c", maprows[y][x]);
        }
        printf("|\n");
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

// the cube configuration
int faces[4][4] = { 0 };
#define TRANSITIONS 14
struct transition {
    int from_face;
    char from_side;
    int to_face;
    char to_side;
    // char rotate_direction;
    int rotate_count; // number of rotations to the right
    char coord_flip;
} transitions[TRANSITIONS];

void determine_faces_configuration(void) {
    // determine the face configuration
    printf("Tile size is %d.\n", tile_size);
    int face_index = 1;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (((y * tile_size) < row_count) &&
                ((x * tile_size) < (int)strlen(maprows[y * tile_size]))) {
                if (maprows[y * tile_size][x * tile_size] != ' ') {
                    faces[y][x] = face_index;
                    face_index += 1;
                }
            }
        }
    }
    assert((face_index - 1) == 6);

    // read transition information from file
    FILE *fin = fopen((tile_size == 4) ? "transitions_4.txt" : "transitions_50.txt", "r");
    int transition_index = 0;
    char line[50];
    while (fgets(line, 50, fin) ) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        int f1, f2, cnt;
        char s1, s2, flip;
        // skip comment and empty lines
        if ((strlen(line) == 0) || (line[0] == ';')) {
            continue;
        } else if (sscanf(line, "%d %c %d %c R %d %c", &f1, &s1, &f2, &s2, &cnt, &flip) == 6) {
            transitions[transition_index].from_face = f1;
            transitions[transition_index].from_side = s1;
            transitions[transition_index].to_face = f2;
            transitions[transition_index].to_side = s2;
            transitions[transition_index].rotate_count = cnt;
            transitions[transition_index].coord_flip = flip;
            transition_index += 1;
        } else {
            printf("Unrecognized info in config file.\n");
        }
    }
    fclose(fin);
    assert(transition_index == TRANSITIONS);
    // verify that all sides connect back to the same
    for (int t = 0; t < TRANSITIONS; t++) {
        for (int t2 = 0; t2 < TRANSITIONS; t2++) {
            if (t != t2) {
                if ((transitions[t].from_face == transitions[t2].to_face) &&
                    (transitions[t].from_side == transitions[t2].to_side)) {
                    assert(transitions[t].to_face == transitions[t2].from_face);
                    assert(transitions[t].to_side == transitions[t2].from_side);
                }
            }
        }
    }
}

void print_faces_configuration(void) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            printf("%c", (faces[y][x] == 0) ? '.' : faces[y][x] + '0');
        }
        printf("\n");
    }
}

int get_face(struct point pos) {
    int face_x = pos.x / tile_size;
    int face_y = pos.y / tile_size;
    assert(faces[face_y][face_x] != 0);
    return faces[face_y][face_x];
}

char get_side(struct point pos, struct point newpos) {
    if (newpos.x > pos.x) return 'R';
    if (newpos.x < pos.x) return 'L';
    if (newpos.y < pos.y) return 'T';
    if (newpos.y > pos.y) return 'B';
    assert(0 && "Cannot determine side");
    return '0';
}

struct point get_face_xy(int face_id) {
    struct point face_xy = { -1, -1 };
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (faces[y][x] == face_id) {
                face_xy.x = x * tile_size;
                face_xy.y = y * tile_size;
            }
        }
    }
    assert((face_xy.x != -1) && (face_xy.y != -1));
    return face_xy;
}

int need_wrap(struct point pos) {
    if ((pos.x < 0) || (pos.y < 0)) return 1;
    if ((pos.y >= row_count) || (pos.x >= max_column)) return 1;
    if (getpos(pos) == ' ') return 1;
    return 0;
}

void perform_wrap(struct point pos, struct point *newpos, int *dir) {
    assert(pos.x >= 0);
    assert(newpos != NULL);
    assert(dir != NULL);

    // determine the 'from' face and side
    int from_face = get_face(pos);
    char from_side = get_side(pos, *newpos);
    int to_face = -1, to_side = '0', rotations = 0, flip = '0';
    for (int t = 0; t < TRANSITIONS; t++) {
        if ((from_face == transitions[t].from_face) &&
            (from_side == transitions[t].from_side)) {
            to_face = transitions[t].to_face;
            to_side = transitions[t].to_side;
            rotations = transitions[t].rotate_count;
            flip = transitions[t].coord_flip;
            break;
        }
    }
    if (debug) printf("Target: face %d, side %c, rotations %d.\n", to_face, to_side, rotations);
    assert((to_face != -1) && "cannot find correct wrap information");
    // calculate the wrapped position and direction
    struct point face_xy = get_face_xy(to_face);
    if (debug) printf("face_xy = %d, %d.\n", face_xy.x, face_xy.y);
    int offset = -1;
    if ((from_side == 'L') || (from_side == 'R')) {
        offset = pos.y % tile_size;
    } else if ((from_side == 'T') || (from_side == 'B')) {
        offset = pos.x % tile_size;
    } else {
        assert(0 && "unknown side");
    }
    if (flip == 'Y') offset = tile_size - 1 - offset;
    switch (to_side) {
    case 'T':
        newpos->x = face_xy.x + offset;
        newpos->y = face_xy.y;
        break;
    case 'B':
        newpos->x = face_xy.x + offset;
        newpos->y = face_xy.y + tile_size - 1;
        break;
    case 'L':
        newpos->x = face_xy.x;
        newpos->y = face_xy.y + offset;
        break;
    case 'R':
        newpos->x = face_xy.x + tile_size - 1;
        newpos->y = face_xy.y + offset;
        break;
    default:
        assert(0 && "unknown 'to' side");
        break;
    }

    *dir = (*dir + rotations) % 4;
}

void step_one(struct point *pos, int *dir) {
    struct point newpos = *pos;
    int newdir = *dir;
    newpos.x += dir_sequence[*dir].x;
    newpos.y += dir_sequence[*dir].y;
    if (need_wrap(newpos)) perform_wrap(*pos, &newpos, &newdir);

    // check on wall
    char c = getpos(newpos);
    if (c == '#') {
        newpos = *pos;
        newdir = *dir;
    } 
    *pos = newpos;
    *dir = newdir;
    setpos(newpos, dir_chars[*dir]);
}

void perform_move(struct move move, struct point *pos, int *dir)  {
    struct point newpos = *pos;
    if (move.type == MOVE_RIGHT) {
        *dir = (*dir + 1) % 4;
        setpos(*pos, dir_chars[*dir]);
    } else if (move.type== MOVE_LEFT) {
        *dir = (*dir + 4 - 1) % 4;
        setpos(*pos, dir_chars[*dir]);
    } else if (move.type == MOVE_COUNT) {
        assert(move.count > 0);
        while (move.count > 0) {
            step_one(&newpos, dir);
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
    tile_size = MAX(row_count, max_column) / 4;
    fillout_rows();
    printf("Map has %d rows.\n", row_count);
    printf("Map has %d columns.\n", max_column);
    printf("Directions is %lu characters long.\n", strlen(directions));

    // implement algorithm
    struct point pos = find_start();
    int dir = 0;
    printf("Start is at (%d, %d), facing '%c'.\n", pos.x, pos.y, dir_chars[dir]);
    maprows[pos.y][pos.x] = dir_chars[dir];
    if (debug) print_map(pos, 30);

    determine_faces_configuration();
    if (debug) print_faces_configuration();

    struct move move = next_move();
    while (move.type != MOVE_NONE) {
        if (debug) print_move(move);
        perform_move(move, &pos, &dir);
        if (debug) print_map(pos, 30);

        move = next_move();
    }
    printf("Position is (%d, %d)\n", pos.x + 1, pos.y + 1);
    printf("Password is %d.\n", 1000 * (pos.y + 1) + 4 * (pos.x + 1) + dir);

    return EXIT_SUCCESS;
}

