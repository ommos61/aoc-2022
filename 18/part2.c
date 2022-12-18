
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
int maxx = 0, maxy = 0, maxz = 0;

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
            assert(v1 >= 0);
            assert(v2 >= 0);
            assert(v3 >= 0);
            maxx = MAX(maxx, v1);
            maxy = MAX(maxy, v2);
            maxz = MAX(maxz, v3);
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

int trapped_count = 0;
struct box trapped[MAX_BOXES];

int *visited;
#define ID_UNKNOWN 0
#define ID_BOX     1
#define ID_FREE    2
int *what;

int is_coord_listed(struct box *list, int count, int x, int y, int z) {
    int found = 0;

    for (int i = 0; i <  count; i++) {
        if ((list[i].x == x) && (list[i].y == y) && (list[i].z == z)) {
            found = 1;
            break;
        }
    }

    return found;
}

int find_box(struct box *box, struct dir *dir) {
    int found = is_coord_listed(boxes, box_count,
            box->x + dir->x, box->y + dir->y, box->z + dir->z);

    return found;
}

int find_trapped(struct box *coord, struct dir *dir) {
    int found = is_coord_listed(trapped, trapped_count,
            coord->x + dir->x, coord->y + dir->y, coord->z + dir->z);

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
            } else if (find_trapped(boxes + i, dirs + d)) {
                boxes[i].sides -= 1;
            }
        }
    }
}

void SET(int *space, int x, int y, int z, int value) {
    int offset = z * (maxy + 1) * (maxx + 1) + y * (maxx + 1) + x;
    *(space + offset) = value;
}

int GET(int *space, int x, int y, int z) {
    int offset = z * (maxy + 1) * (maxx + 1) + y * (maxx + 1) + x;
    return *(space + offset);
}

void walk_free(int x, int y, int z) {
    if ((x >= 0) && (x <= maxx) &&
        (y >= 0) && (y <= maxy) &&
        (z >= 0) && (z <= maxz) &&
        (! GET(visited, x, y, z))) {
//        (GET(what, x, y, z) == ID_FREE)) {
        SET(visited, x, y, z, 1);
        if (GET(what, x, y, z) != ID_BOX) {
            SET(what, x, y, z, ID_FREE);
        }
        for (unsigned int d = 0; d <= array_count(dirs); d++) {
            int newx = x + dirs[d].x;
            int newy = y + dirs[d].y;
            int newz = z + dirs[d].z;

            walk_free(newx, newy, newz);
        }
    }
}

void determine_trapped_spaces(void) {
    int size = (maxx + 1) * (maxy + 1) * (maxz + 1);
    visited = malloc(size * sizeof(int));
    memset(visited, 0, size * sizeof(int));
    what = malloc(size * sizeof(int));
    memset(what, 0, sizeof(int));

    // fill in the actual boxes
    for (int i = 0; i < box_count; i++) {
        SET(what, boxes[i].x, boxes[i].y, boxes[i].z, ID_BOX);
        SET(visited, boxes[i].x, boxes[i].y, boxes[i].z, 1);
    }

    // now for the outside, make the empty spaces free
    for (int x = 0; x <= maxx; x++) {
        for (int y = 0; y <= maxy; y++) {
            for (int z = 0; z <= maxz; z++) {
                if (((z == 0) || (z == maxz)) && GET(what, x, y, z) != ID_BOX) {
                    SET(what, x, y, z, ID_FREE);
                }
                if (((y == 0) || (y == maxy)) && GET(what, x, y, z) != ID_BOX) {
                    SET(what, x, y, z, ID_FREE);
                }
                if (((x == 0) || (x == maxx)) && GET(what, x, y, z) != ID_BOX) {
                    SET(what, x, y, z, ID_FREE);
                }
            }
        }
    }
    // for all free (ID_FREE) boxes, walk all directions that are not
    // boxes and mark the ID_FREE also
    for (int x = 0; x <= maxx; x++) {
        for (int y = 0; y <= maxy; y++) {
            for (int z = 0; z <= maxz; z++) {
                if ((GET(what, x, y, z) == ID_FREE) && (! GET(visited, x, y, z))) {
                    walk_free(x, y, z);
                }
            }
        }
    }

    // Add all ID_UNKNOWN ones to the trapped list
    for (int x = 0; x <= maxx; x++) {
        for (int y = 0; y <= maxy; y++) {
            for (int z = 0; z <= maxz; z++) {
                if (GET(what, x, y, z) == ID_UNKNOWN) {
                    trapped[trapped_count].x = x;
                    trapped[trapped_count].y = y;
                    trapped[trapped_count].z = z;
                    trapped_count += 1;
                }
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
    printf("Max coordiante is (%d, %d, %d).\n", maxx, maxy, maxz);

    // implement algorithm
    determine_trapped_spaces();
    printf("Detected %d trapped spaces.\n", trapped_count);

    determine_free_sides();
    int count = 0;
    for (int i = 0; i < box_count; i++) {
        count += boxes[i].sides;
    }
    printf("There are %d free sides.\n", count);

    return EXIT_SUCCESS;
}

