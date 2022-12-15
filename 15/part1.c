
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 1; // 0 = no debug output, 1 = some extra debug output

struct point {
    int x;
    int y;
};

int sensor_count = 0;
#define MAX_SENSORS 50
struct sensor {
    struct point location;
    struct point beacon;
} sensors[MAX_SENSORS];

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

        int sx, sy, bx, by;
        if (sscanf(line, "Sensor at x=%d, y=%d: closest beacon is at x=%d, y=%d", &sx, &sy, &bx, &by) == 4) {
            // parse the data
            sensors[sensor_count].location.x = sx;
            sensors[sensor_count].location.y = sy;
            sensors[sensor_count].beacon.x = bx;
            sensors[sensor_count].beacon.y = by;
            sensor_count += 1;
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

struct range {
    int start;
    int end;
    struct range *next;
};

int distance(struct point p1, struct point p2) {
    int distance = abs(p1.x - p2.x) + abs(p1.y - p2.y);

    return distance;
}

void print_ranges(struct range *ranges) {
    struct range *r = ranges;
    while (r != NULL) {
        printf("(%d->%d) ", r->start, r->end);
        r = r->next;
    }
    printf("\n");
}

void add_range_sorted(struct range **p_ranges, int start, int end) {
    if (*p_ranges == NULL) {
        struct range *r = malloc(sizeof(struct range));
        r->start = start;
        r->end = end;
        r->next = NULL;
        *p_ranges = r;
    } else {
        struct range *r = malloc(sizeof(struct range));
        r->start = start;
        r->end = end;
        r->next = NULL;
        struct range *previous = *p_ranges;
        struct range *current = *p_ranges;
        while ((current != NULL) && (current->start < start)) {
            if (current != previous) {
                previous = previous->next;
            }
            current = current->next;
        }
        if (current == *p_ranges) {
            // insert at start
            r->next = *p_ranges;
            *p_ranges = r;
        } else {
            r->next = previous->next;
            previous->next = r;
        }
    }
}

int is_beacon(int x, int y) {
    int is_beacon = 0;

    for (int s = 0; s < sensor_count; s++) {
        if ((sensors[s].beacon.x == x) && (sensors[s].beacon.y == y)) {
            is_beacon = 1;
            break;
        }
    }

    return is_beacon;
}

int count_covered(int row) {
    int count = 0;
    struct range *ranges = NULL;

    for (int s = 0; s < sensor_count; s++) {
        int beacon_distance = distance(sensors[s].location, sensors[s].beacon);
        if (abs(row - sensors[s].location.y) < beacon_distance) {
            struct point p = { sensors[s].location.x, row };
            while (distance(p, sensors[s].location) < beacon_distance) {
                p.x += 1;
            }
            int x_dist = p.x - sensors[s].location.x;
            add_range_sorted(&ranges, sensors[s].location.x - x_dist, sensors[s].location.x + x_dist);
        }
    }

    struct range *r = ranges;
    int last_end = INT_MIN;
    while (r != NULL) {
        if (r->start > last_end) {
            // there was no overlap with previous range
            for (int x = r->start; x <= r->end; x++) {
                if (! is_beacon(x, row)) {
                    count += 1;
                }
            }
            last_end = r->end;
        } else {
            // there is overlap with previous range
            if (r->end > last_end) {
                for (int x = last_end + 1; x <= r->end; x++) {
                    if (! is_beacon(x, row)) {
                        count += 1;
                    }
                }
                last_end = r->end;
            } else {
                // completely overlapping, so ignore
            }
        }

        r = r->next;
    }

    return count;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }
    int sample_input = 0;
    if (strcmp(fname, "input.txt") != 0) {
        sample_input = 1;
    }

    readData(fname);
    printf("Sensor count: %d\n", sensor_count);

    // implement algorithm
    int row = 2000000;
    if (sample_input) row = 10;
    int covered = count_covered(row);
    printf("On row y=%d, there are %d positions where no beacon can be present.\n", row, covered);

    return EXIT_SUCCESS;
}

