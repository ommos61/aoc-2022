
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

// Data
int row_count = 0;
int column_count = 0;
#define MAX_ROWS 200
int *rows[MAX_ROWS];

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

        int value = 0;
        column_count = strlen(line);
        if (sscanf(line, "%d", &value) == 1) {
            // TODO: parse the data
            rows[row_count] = malloc(strlen(line) * sizeof(int));
            for (unsigned int i = 0; i < strlen(line); i++) {
                rows[row_count][i] = line[i] - '0';
            }
            row_count++;
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

void print_patch(void) {
    for (int y = 0; y < row_count; y++) {
        for (int x = 0; x < column_count; x++) {
            printf("%d", rows[y][x]);
        }
        printf("\n");
    }
}

int get_scenic_score(int xpos, int ypos, int debug) {
    int score = 1;
    assert((xpos >= 0) && (ypos >= 0));

    int height = rows[ypos][xpos];
    // look up
    if (debug) printf("(");
    for (int y = ypos - 1; y >= 0; y--) {
        if ((rows[y][xpos] >= height) || (y == 0)) {
            score *= ypos - y;
            break;
        }
    }
    if (debug) printf("%d ", score);
    // look down
    for (int y = ypos + 1; y < row_count; y++) {
        if ((rows[y][xpos] >= height) || (y == row_count - 1)) {
            score *= y - ypos;
            break;
        }
    }
    if (debug) printf("%d ", score);
    // look left
    for (int x = xpos - 1; x >= 0; x--) {
        if ((rows[ypos][x] >= height) || (x == 0)) {
            score *= xpos - x;
            break;
        }
    }
    if (debug) printf("%d ", score);
    // look right
    for (int x = xpos + 1; x < column_count; x++) {
        if ((rows[ypos][x] >= height) || (x == column_count - 1)) {
            score *= x - xpos;
            break;
        }
    }
    if (debug) printf("%d)", score);

    return score;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    //print_patch();

    // TODO: implement algorithm
    int score = -1;
    int xpos = -1, ypos = -1;
    printf("--------------------\n");
    for (int y = 1; y < row_count - 1; y++) {
        for (int x = 1; x < column_count - 1; x++) {
            int debug = 0;
            if ((x == 2) && (y == 1)) debug = 0;
            int new_score = get_scenic_score(x, y, debug);
            if (new_score > score) {
                score = new_score;
                xpos = x; ypos = y;
            }
            //printf("%3d ", new_score);
        }
        //printf("\n");
    }
    printf("Highest scenic score at (%d, %d): %d\n", xpos, ypos, score);

    return EXIT_SUCCESS;
}

