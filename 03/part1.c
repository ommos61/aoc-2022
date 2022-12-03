
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

int sacks_count = 0;
char *sacks[3000] = {0};

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

        char *content = malloc(strlen(line) + 1);
        if (sscanf(line, "%s", content) == 1) {
            // TODO: parse the data
            assert((strlen(content) % 2 == 0) && "Content must be a multiple of 2");
            //printf("Content: %s\n", content);
            sacks[sacks_count] = content;
            sacks_count++;
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

char find_wrong_item(char *content) {
    char wrong = 0;

    int half = strlen(content) / 2;
    for (int i = 0; i < half; i++) {
        for (int j = 0; j < half; j++) {
            if (content[i] == content[half + j]) {
                wrong = content[i];
                break;
            }
        }
        if (wrong != 0) break;
    }

    assert((wrong != 0) && "There is no double item");
    return wrong;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // TODO: implement algorithm
    int priority = 0;
    for (int i = 0; i < sacks_count; i++) {
        char wrong = find_wrong_item(sacks[i]);
        //printf("Wrong item: %c\n", wrong);
        if ((wrong >= 'a') && (wrong <= 'z')) {
            priority += (wrong - 'a' + 1);
        } else if ((wrong >= 'A') && (wrong <= 'Z')) {
            priority += (wrong - 'A' + 27);
        } else {
            fprintf(stderr, "Unknown item '%c' in sack %d\n", wrong, i);
        }
    }
    printf("Total priority: %d\n", priority);

    return EXIT_SUCCESS;
}

