
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
int debug = 1; // 0 = no debug output, 1 = some extra debug output

int move_count = 0;
#define MAX_MOVES 5000
struct move {
    char direction;
    int count;
} moves[MAX_MOVES] = {0};

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

int get_visited(int *visited, struct field info, int x, int y) {
    return *(visited + (info.width * y) + x);
}

void set_visited(int *visited, struct field info, int x, int y) {
    *(visited + (info.width * y) + x) = 1;
}

void print_visited(int * visited, struct field info) {
    for (int y = info.height - 1; y >= 0; y--) {
        for (int x = 0; x < info.width; x++) {
            if ((x == info.startx) && (y == info.starty)) {
                printf("s");
            } else {
                printf("%c", get_visited(visited, info, x, y) ? '#' : '.');
            }
        }
        printf("\n");
    }
}

void move_tail(int headx, int heady, int *tailx, int *taily) {
    if (heady == *taily) { // same row
        if (abs(headx - *tailx) > 1) {
            if (*tailx < headx) {
                (*tailx)++;
            } else {
                (*tailx)--;
            }
        }
    } else if (headx == *tailx) { // same column
        if (abs(heady - *taily) > 1) {
            if (*taily < heady) {
                (*taily)++;
            } else {
                (*taily)--;
            }
        }
    } else { // diagonal
        if (abs(headx - *tailx) > 1) {
            *tailx += (*tailx < headx) ? 1 : -1;
            *taily = heady;
        } else if (abs(heady - *taily) > 1) {
            *tailx = headx;
            *taily += (*taily < heady) ? 1 : -1;
        }
    }
}

void simulate_moves(int *visited, struct field info) {
    int headx = info.startx, heady = info.starty;
    int tailx = info.startx, taily = info.starty;

    set_visited(visited, info, tailx, taily);
    for (int i = 0; i < move_count; i++) {
        for (int j = 0; j < moves[i].count; j++) {
            // Move the head
            switch (moves[i].direction) {
            case 'U':
                heady++; break;
            case 'D':
                heady--; break;
            case 'R':
                headx++; break;
            case 'L':
                headx--; break;
            default:
                assert(0 && "Illegal move"); break;
            }

            // Move the tail
            move_tail(headx, heady, &tailx, &taily);
            //printf("Tail: (%d, %d)\n", tailx, taily);
            set_visited(visited, info, tailx, taily);
        }
        //printf("---> %c %d\n", moves[i].direction, moves[i].count);
        //print_visited(visited, info);
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

    simulate_moves(visited, field_info);
    int visited_count = 0;
    for (int i = 0; i < size; i++) {
        if (visited[i]) visited_count++;
    }
    printf("The simulation visited %d fields\n", visited_count);

    return EXIT_SUCCESS;
}

