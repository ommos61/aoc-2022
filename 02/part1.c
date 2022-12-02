
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

#define MAX_STRATEGIES 3000
int strategy_count = 0;
struct strategy {
    char opponent;
    char self;
} strategies[MAX_STRATEGIES];

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
        //if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        char v1, v2;
        if (sscanf(line, "%c %c", &v1, &v2) == 2) {
            // TODO: parse the data
            strategies[strategy_count].opponent = v1;
            strategies[strategy_count].self = v2;
            strategy_count++;
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

int get_score(char opponent, char self) {
    int score = self - 'X' + 1;

    if ((opponent - 'A') == (self - 'X')) {
        score += 3;
    } else if ((self == 'X') && (opponent == 'C')) {
        score += 6;
    } else if ((self == 'Z') && (opponent == 'B')) {
        score += 6;
    } else if ((self == 'Y') && (opponent == 'A')) {
        score += 6;
    }
    return score;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
//    printf("Strategies: %d\n", strategy_count);
//    printf("First: %c -> %c\n", strategies[0].opponent, strategies[0].self);
//    printf("Last: %c -> %c\n", strategies[strategy_count-1].opponent, strategies[strategy_count-1].self);

    // TODO: implement algorithm
    int total_score = 0;
    for (int i = 0; i < strategy_count; i++) {
        int score = get_score(strategies[i].opponent, strategies[i].self);
        total_score += score;
    }
    printf("Total Score: %d\n", total_score);

    return EXIT_SUCCESS;
}

