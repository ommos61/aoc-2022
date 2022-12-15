
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
int beacon_count = 0;
struct point beacons[MAX_SENSORS];

void add_beacon(int x, int y) {
    int found = 0;

    for (int i = 0; i < beacon_count; i++) {
        if ((beacons[i].x == x) && (beacons[i].y == y)) {
            found = 1;
            break;
        }
    }

    if (! found) {
        beacons[beacon_count].x = x;
        beacons[beacon_count].y = y;
        beacon_count += 1;
    }
}

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
            add_beacon(bx, by);
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

    for (int b = 0; b < sensor_count; b++) {
        if ((beacons[b].x == x) && (beacons[b].y == y)) {
            is_beacon = 1;
            break;
        }
    }

    return is_beacon;
}

void collaps_ranges(struct range *ranges) {
    struct range *prev = NULL;
    struct range *curr = ranges;

    while (curr != NULL) {
        if (prev != NULL) {
            if (curr->start <= prev->end + 1) {
                prev->end = MAX(prev->end, curr->end);
                prev->next = curr->next;
                free(curr);
                curr = prev->next;
                continue;
            }
        }

        // update for next iteration
        if (prev == NULL) prev = curr; else prev = prev->next;
        curr = curr->next;
    }
}

struct range *get_covered(int row) {
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

    //printf("Before collaps: "); print_ranges(ranges);
    collaps_ranges(ranges);
    //printf("After collaps:  "); print_ranges(ranges);

    return ranges;
}

int find_beacon(int row) {
    int x = -1;
    struct range *ranges = NULL;

    for (int s = 0; s < sensor_count; s++) {
        int beacon_distance = distance(sensors[s].location, sensors[s].beacon);
        int remain_distance = beacon_distance - abs(row - sensors[s].location.y);
        if (remain_distance > 0) {
            add_range_sorted(&ranges,
                    sensors[s].location.x - remain_distance,
                    sensors[s].location.x + remain_distance);
        }
    }
    collaps_ranges(ranges);
    if (ranges->next != NULL) {
        x = ranges->end + 1;
    }

    return x;
}

int is_in_range(struct range *ranges, int x) {
    int found = 0;

    struct range *r = ranges;
    while (r != NULL) {
        if ((x >= r->start) && (x <= r->end)) {
            found = 1;
            break;
        }
        r = r->next;
    }

    return found;
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
    printf("Beacon count: %d\n", beacon_count);

    // implement algorithm
    int coord_max = 4000000;
    if (sample_input) coord_max = 20;
    int found = 0;
    for (int y = 0; y <= coord_max; y++) {
        printf("\rLine: %d                   ", y);
        int x = find_beacon(y);
        if (x != -1) {
            long frequency = 4000000L * x + y;
            printf("\nFound beacon at (%d, %d) with frequency %ld.\n", x, y, frequency);
            found = 1;
        }
#if 0
        struct range *ranges = get_covered(y);
        if (ranges->next != NULL) {
            printf("\nFound line %d with multiple ranges.\n", y);
            found = 1;
            print_ranges(ranges);
        }
#if 0
        for (int x = 0; x <= coord_max; x++) {
            if (! is_in_range(ranges, x)) {
                int frequency = 4000000 * x + y;
                printf("\nFound beacon at (%d, %d) with frequency %d.\n", x, y, frequency);
                found = 1;
            }
        }
#endif
#endif
    }
    if (! found) printf("\nDidn't find any active beacon!!!!\n"); else printf("\n");
    printf("The frequence %d was too low.\n", 277092432);
    printf("The problem with that number was that it wrapped around as a standard integer.\n");

    return EXIT_SUCCESS;
}

