
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

int is_in_group(char item, char *content) {
    int result = 0;

    for (unsigned int i = 0; i < strlen(content); i++) {
        if (item == content[i]) {
            result = 1;
            break;
        }
    }

    return result;
}

char find_common_item(int group) {
    char common = 0;

    int offset = group * 3;
    for (unsigned int i = 0; i < strlen(sacks[offset]); i++) {
        char item = sacks[offset][i];
        if (is_in_group(item, sacks[offset + 1]) && is_in_group(item, sacks[offset + 2])) {
            common = item;
            break;
        }
    }

    assert((common != 0) && "There doesn't seem to be a common item in this group");
    return common;
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
    for (int i = 0; i < sacks_count / 3; i++) {
        char common = find_common_item(i);
        //printf("Common item: %c\n", common);
        if ((common >= 'a') && (common <= 'z')) {
            priority += (common - 'a' + 1);
        } else if ((common >= 'A') && (common <= 'Z')) {
            priority += (common - 'A' + 27);
        } else {
            fprintf(stderr, "Unknown item '%c' in sack %d\n", common , i);
        }
    }
    printf("Total priority: %d\n", priority);

    return EXIT_SUCCESS;
}

