
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

#define MAX_ELVES 500
int elves[MAX_ELVES] = {0};
int elves_count = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    int current_elf = 0;
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        int value = 0;
        if (sscanf(line, "%d", &value) == 1) {
            // TODO: parse the data
            elves[current_elf] += value;
        } else if (errno != 0) {
            perror("sscanf");
        } else {
            current_elf++;
            //fprintf(stderr, "Unexpected input format '%s'.\n", line);
        }

        line_count++;
    }
    elves_count = current_elf;

    printf("lines = %d\n", line_count);
    fclose(fin);
}

int get_lowest_elf(int *list) {
    int result = elves[list[0]] <= elves[list[1]] ? 0 : 1;
    result = elves[list[result]] <= elves[list[2]] ? result : 2;

    return result;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // List all elves
    int max_elf = 0;
    int max_elves[] = { 0, 1, 2 };
    int elves_total = elves[0] + elves[1] + elves[2];
    for (int i = 1; i <= elves_count; i++) {
        if (elves[i] > elves[max_elf]) {
            max_elf = i;
        }
        if (i > 2) {
            int lowest = get_lowest_elf(max_elves);
            if (elves[max_elves[lowest]] < elves[i]) {
                elves_total = elves_total - elves[max_elves[lowest]] + elves[i];
                max_elves[lowest] = i;
            }
        }
    }
    printf("Max elf %d: %d\n", max_elf, elves[max_elf]);
    printf("Max three elves: %d\n", elves_total);

    return EXIT_SUCCESS;
}

