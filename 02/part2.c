
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
    char result;
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
            strategies[strategy_count].result = v2;
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

// Rock = 1, Paper = 2, Scissors = 3
// Rock > Scissors, Scissors > Paper, Paper > Rock
int loses[3] = { 3, 1, 2 };
int draws[3] = { 1, 2, 3 };
int wins[3] = { 2, 3, 1 };

int get_score(char opponent, char result) {
    int score = (result - 'X') * 3;

    if (result == 'X') { // Lose
        score += loses[opponent - 'A'];
    } else if (result == 'Y') { // Draw
        score += draws[opponent - 'A'];
    } else if (result == 'Z') { // Win
        score += wins[opponent - 'A'];
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
//    printf("First: %c -> %c\n", strategies[0].opponent, strategies[0].result);
//    printf("Last: %c -> %c\n", strategies[strategy_count-1].opponent, strategies[strategy_count-1].result);

    // TODO: implement algorithm
    int total_score = 0;
    for (int i = 0; i < strategy_count; i++) {
        int score = get_score(strategies[i].opponent, strategies[i].result);
        total_score += score;
    }
    printf("Total Score: %d\n", total_score);

    return EXIT_SUCCESS;
}

