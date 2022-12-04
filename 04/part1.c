
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

int pair_count = 0;
#define MAX_PAIRS 3000
struct pair {
    int start1;
    int end1;
    int start2;
    int end2;
} pairs[MAX_PAIRS] = {0};

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

        int s1 = 0;
        int e1 = 0;
        int s2 = 0;
        int e2 = 0;
        if (sscanf(line, "%d-%d,%d-%d", &s1, &e1, &s2, &e2) == 4) {
            // TODO: parse the data
            pairs[pair_count].start1 = s1;
            pairs[pair_count].end1 = e1;
            pairs[pair_count].start2 = s2;
            pairs[pair_count].end2 = e2;
            pair_count++;
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

int do_overlap(struct pair p) {
    int result = 0;

    if ((p.start1 <= p.start2) && (p.end1 >= p.end2)) {
        result = 1;
    } else if ((p.start1 >= p.start2) && (p.end1 <= p.end2)) {
        result = 1;
    }
    return result;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("Pairs: %d\n", pair_count);

    // TODO: implement algorithm
    int overlap_count = 0;
    for (int i = 0; i < pair_count; i++) {
        if (do_overlap(pairs[i])) {
            overlap_count++;
        }
    }
    printf("Overlapping pairs: %d\n", overlap_count);

    return EXIT_SUCCESS;
}

