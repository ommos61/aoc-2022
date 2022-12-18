
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

int box_count = 0;
#define MAX_BOXES 3000
struct box {
    int x;
    int y;
    int z;
    int sides;
} boxes[MAX_BOXES];

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

        int v1, v2, v3;
        if (sscanf(line, "%d,%d,%d", &v1, &v2, &v3) == 3) {
            // parse the data
            boxes[box_count].x = v1;
            boxes[box_count].y = v2;
            boxes[box_count].z = v3;
            box_count += 1;
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

struct dir {
    int x; int y; int z;
} dirs[] = {
    { 1, 0, 0 },
    { -1, 0, 0 },
    { 0, 1, 0 },
    { 0, -1, 0 },
    { 0, 0, 1 },
    { 0, 0, -1 },
};

int find_box(struct box *box, struct dir *dir) {
    int found = 0;

    for (int i = 0; i < box_count; i++) {
        if ((boxes[i].x == box->x + dir->x) &&
            (boxes[i].y == box->y + dir->y) &&
            (boxes[i].z == box->z + dir->z)) {
            found = 1;
            break;
        }
    }

    return found;
}

void determine_free_sides(void) {
    // initialize the free sides to max
    for (int i = 0; i < box_count; i++) {
        boxes[i].sides = 6;
    }

    for (int i = 0; i < box_count; i++) {
        if (debug) printf("Handling box %d.\n", i + 1);
        for (unsigned int d = 0; d < array_count(dirs); d++) {
            if (find_box(boxes + i, dirs + d)) {
                boxes[i].sides -= 1;
            }
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
    printf("There are %d boxes.\n", box_count);

    // TODO: implement algorithm
    determine_free_sides();
    int count = 0;
    for (int i = 0; i < box_count; i++) {
        count += boxes[i].sides;
    }
    printf("There are %d free sides.\n", count);

    return EXIT_SUCCESS;
}

