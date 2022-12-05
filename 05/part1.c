
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

int stack_count = 0;
#define MAX_STACKS 100
char *stacks[MAX_STACKS] = { NULL };
#define MAX_STACK_DEPTH 1024

int move_count = 0;
#define MAX_MOVES 1024
struct move {
    int count;
    int from;
    int to;
} moves[MAX_MOVES] = { 0 };

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    int reading_procedure = 0;
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (! reading_procedure) {
            // read the stack contents
            if (line[1] == '1') {
                // end of stacks information
                reading_procedure = 1;
            } else {
                // reading the stacks information
                int stack_index = 0;
                unsigned int line_index = 0;
                while (line_index < strlen(line)) {
                    if (stacks[stack_index] == NULL) {
                        stacks[stack_index] = malloc(MAX_STACK_DEPTH);
                        memset(stacks[stack_index], 0, MAX_STACK_DEPTH);
                    }
                    if (line[line_index + 1] != ' ') {
                        char insert = line[line_index + 1];
                        int index = 0;
                        while (stacks[stack_index][index] != 0) {
                            char tmp = stacks[stack_index][index];
                            stacks[stack_index][index] = insert;
                            insert = tmp;

                            index++;
                        }
                        stacks[stack_index][index] = insert;
                    }
                    stack_index++;
                    line_index += 4;
                }
                if (stack_index > stack_count) {
                    stack_count = stack_index;
                }
            }
        } else {
            // read the moves procedure
            int count, from, to;
            if (strlen(line) == 0) {
                // skip an empty line
            } else if (sscanf(line, "move %d from %d to %d", &count, &from, &to) == 3) {
                moves[move_count].count = count;
                moves[move_count].from = from - 1;
                moves[move_count].to = to - 1;
                move_count++;
            } else {
                fprintf(stderr, "Error: unknown format in line %d: '%s'\n", line_count++, line);
            }
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    printf("stack count = %d\n", stack_count);
    printf("move count = %d\n", move_count);
    fclose(fin);
}

void print_stacks(void) {
    for (int i = 0; i < stack_count; i++) {
        printf("%-3d: ", i + 1);
        for (unsigned int j = 0; j < strlen(stacks[i]); j++) {
            printf("%c ", stacks[i][j]);
        }
        printf("\n");
    }
}

void perform_move(unsigned int count, unsigned int from, unsigned int to) {
    assert((strlen(stacks[from]) >= count) && "there are not enough items on the stack to move");
    assert((strlen(stacks[to]) + count < MAX_STACK_DEPTH) && "a stack is getting too big");

    // now actually move the items
    for (unsigned int i = 0; i < count; i++) {
        strcat(stacks[to], stacks[from] + (strlen(stacks[from]) - 1));
        stacks[from][strlen(stacks[from]) - 1] = 0;
    }
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // TODO: implement algorithm
    print_stacks();
    for (int i = 0; i < move_count; i++) {
        //printf("move %d: %d from %d to %d\n", i, moves[i].count, moves[i].from + 1, moves[i].to + 1);
        perform_move(moves[i].count, moves[i].from, moves[i].to);
    }
    printf("Result after the moves:\n");
    print_stacks();
    printf("top stacks: '");
    for (int i = 0; i < stack_count; i++) {
        printf("%c", stacks[i][strlen(stacks[i]) - 1]);
    }
    printf("'\n");

    return EXIT_SUCCESS;
}

