
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

int is_visible(int xpos, int ypos) {
    int visible = 0;
    assert((xpos > 0) && (ypos > 0));

    // Walk the row
    int highest = -1;
    for (int x = 0; x < column_count; x++) {
        if (x == xpos) {
            if (highest < rows[ypos][xpos]) {
                visible = '<';
                break;
            }
            highest = -1;
        } else {
            highest = MAX(highest, rows[ypos][x]);
        }
    }
    if (highest < rows[ypos][xpos]) {
        visible = '>';
    }

    // if still not visible, walk the column
    if (!visible) {
        int highest = -1;
        for (int y = 0; y < row_count; y++) {
            if (y == ypos) {
                if (highest < rows[y][xpos]) {
                    visible = '^';
                    break;
                }
                highest = -1;
            } else {
                highest = MAX(highest, rows[y][xpos]);
            }
        }
        if (highest < rows[ypos][xpos]) {
            visible = 'v';
        }
    }

    return visible;
}

int calculate_visible(int do_print) {
    int visible = 0;

    if (do_print) printf("-----------------\n");
    for (int y = 0; y < row_count; y++) {
        for (int x = 0; x < column_count; x++) {
            char t = '.';
            if ((y == 0) || (y == row_count - 1) || (x == 0) || (x == column_count - 1)) {
                visible++;
                t = 'O';
            } else {
                int v = is_visible(x, y);
                if (v) {
                    visible++;
                    t = v;
                }
            }
            if (do_print) printf("%c", t);
        }
        if (do_print) printf("\n");
    }

    return visible;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    // print_patch();

    // TODO: implement algorithm
    int visible = calculate_visible(0);
    printf("Visible trees: %d\n", visible);

    return EXIT_SUCCESS;
}

