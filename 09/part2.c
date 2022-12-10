
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

int move_count = 0;
#define MAX_MOVES 5000
struct move {
    char direction;
    int count;
} moves[MAX_MOVES] = {0};
struct knot {
    int x;
    int y;
};

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

        char direction = 0;
        int value = 0;
        if (sscanf(line, "%c %d", &direction, &value) == 2) {
            // TODO: parse the data
            moves[move_count].direction = direction;
            moves[move_count].count = value;
            move_count++;
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

struct field {
    int width;
    int height;
    int startx;
    int starty;
};
struct field calculate_field_size(void) {
    struct field result = { 0 };

    int minx = 0, maxx = 0, miny = 0, maxy = 0;
    int x = 0, y = 0;
    for (int i = 0; i < move_count; i++) {
        switch (moves[i].direction) {
        case 'U':
            y += moves[i].count;
            break;
        case 'D':
            y -= moves[i].count;
            break;
        case 'R':
            x += moves[i].count;
            break;
        case 'L':
            x -= moves[i].count;
            break;
        default:
            fprintf(stderr, "Unknown move direction '%c'\n", moves[i].direction);
            break;
        }
        if (x < minx) minx = x;
        if (x > maxx) maxx = x;
        if (y < miny) miny = y;
        if (y > maxy) maxy = y;
    }

    result.width = maxx - minx + 1;
    result.height = maxy - miny + 1;
    result.startx = -minx;
    result.starty = -miny;
    return result;
}

int get_visited(int *visited, struct field info, struct knot pos) {
    return *(visited + (info.width * pos.y) + pos.x);
}

void set_visited(int *visited, struct field info, struct knot pos) {
    *(visited + (info.width * pos.y) + pos.x) = 1;
}

void print_visited(int * visited, struct field info) {
    for (int y = info.height - 1; y >= 0; y--) {
        for (int x = 0; x < info.width; x++) {
            if ((x == info.startx) && (y == info.starty)) {
                printf("s");
            } else {
                struct knot pos = { x, y };
                printf("%c", get_visited(visited, info, pos) ? '#' : '.');
            }
        }
        printf("\n");
    }
}

void move_tail(struct knot head, struct knot *tail) {
    if (head.y == tail->y) { // same row
        if (abs(head.x - tail->x) > 1) {
            if (tail->x < head.x) {
                (tail->x)++;
            } else {
                (tail->x)--;
            }
        }
    } else if (head.x == tail->x) { // same column
        if (abs(head.y - tail->y) > 1) {
            if (tail->y < head.y) {
                (tail->y)++;
            } else {
                (tail->y)--;
            }
        }
    } else { // diagonal
        if (abs(head.x - tail->x) > 1) {
            tail->x += (tail->x < head.x) ? 1 : -1;
            tail->y += (tail->y < head.y) ? 1 : -1;
        } else if (abs(head.y - tail->y) > 1) {
            tail->x += (tail->x < head.x) ? 1 : -1;
            tail->y += (tail->y < head.y) ? 1 : -1;
        }
    }
}

void print_field(struct field info, struct knot *knots, int knot_count) {
    for (int y = info.height - 1; y >= 0; y--) {
        for (int x = 0; x < info.width; x++) {
            char c = '.';
            for (int i = 0; i < knot_count; i++) {
                if ((x == knots[i].x) && (y == knots[i].y)) {
                    if (i == 0) {
                        c = 'H';
                    } else {
                        c = '0' + i;
                    }
                    break;
                }
                if ((c == '.') && (x == info.startx) && (y == info.starty)) {
                    c = 's';
                }
            }
            printf("%c", c);
        }
        printf("\n");
    }
}

void simulate_moves(int *visited, struct field info, struct knot *knots, int knot_count) {
    set_visited(visited, info, knots[knot_count - 1]);
    for (int i = 0; i < move_count; i++) {
        if (debug) printf("---> %c %d\n", moves[i].direction, moves[i].count);
        for (int j = 0; j < moves[i].count; j++) {
            // Move the head
            switch (moves[i].direction) {
            case 'U':
                knots[0].y++; break;
            case 'D':
                knots[0].y--; break;
            case 'R':
                knots[0].x++; break;
            case 'L':
                knots[0].x--; break;
            default:
                assert(0 && "Illegal move"); break;
            }

            // Move the tail knots
            for (int k = 1; k < knot_count; k++) {
                move_tail(knots[k - 1], &knots[k]);
            }
            //printf("Tail: (%d, %d)\n", tailx, taily);
            set_visited(visited, info, knots[knot_count - 1]);
            if (debug) print_field(info, knots, knot_count);
        }
    }
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // TODO: implement algorithm
    printf("Moves: %d\n", move_count);
    struct field field_info = calculate_field_size();
    printf("Bridge size is (%d, %d)\n", field_info.width, field_info.height);
    printf("Start at (%d, %d)\n", field_info.startx, field_info.starty);

    int size = field_info.width * field_info.height;
    int *visited = malloc(size * sizeof(int));
    memset(visited, 0, size * sizeof(int));

    int knot_count = 10;
    struct knot knots[knot_count];
    for (int i = 0; i < knot_count; i++) {
        knots[i].x = field_info.startx;
        knots[i].y = field_info.starty;
    }
    simulate_moves(visited, field_info, knots, knot_count);
    if (debug) printf("---> visited\n");
    if (debug) print_visited(visited, field_info);
    int visited_count = 0;
    for (int i = 0; i < size; i++) {
        if (visited[i]) visited_count++;
    }
    printf("The simulation visited %d fields\n", visited_count);
    printf("Info: the answer 2672 is too high\n");

    return EXIT_SUCCESS;
}

