
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

int instruction_count = 0;
#define MAX_INSTRUCTIONS 5000
#define OP_ADDX 1
#define OP_NOOP 2
struct instruction {
    int command;
    int value;
} instructions[MAX_INSTRUCTIONS];

struct cpu_state {
    int X;              // the single register
    int pc;             // the current instruction index
    int cycle;          // the current cycle number
} cpu_state;

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

        int value = 0;
        if (sscanf(line, "addx %d", &value) == 1) {
            // TODO: parse the data
            instructions[instruction_count].command = OP_ADDX;
            instructions[instruction_count].value = value;
            instruction_count++;
        } else if (strcmp(line, "noop") == 0) {
            instructions[instruction_count].command = OP_NOOP;
            instructions[instruction_count].value = 0;
            instruction_count++;
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

void print_state(struct cpu_state *state) {
    printf("%4d: X = %6d, ", state->cycle, state->X);
    switch (instructions[state->pc].command) {
    case OP_ADDX:
        printf("advx %d", instructions[state->pc].value);
        break;
    case OP_NOOP:
        printf("noop");
        break;
    default:
        printf("---> Unknown instruction <---");
        break;
    }
    printf("\n");
}

int execute(struct cpu_state *state, int sample_cycle) {
    int result = 0;
    assert(state != NULL);
    assert(sample_cycle >= 0);

    while (state->cycle <= sample_cycle) {
        assert((state->pc < instruction_count) && "The program is not long enough");
//        print_state(state);
        switch (instructions[state->pc].command) {
        case OP_ADDX:
            state->cycle += 2;
            if (state->cycle >= sample_cycle) {
                result = sample_cycle * state->X;
            }
            state->X += instructions[state->pc].value;
            break;
        case OP_NOOP:
            if (state->cycle == sample_cycle) {
                result = sample_cycle * state->X;
            }
            state->cycle += 1;
            break;
        default:
            printf("--> Unknown instruction\n");
            break;
        }
        state->pc += 1;
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
    printf("Instruction count: %d\n", instruction_count);

    // TODO: implement algorithm
    int sample_points[] = { 20, 60, 100, 140, 180, 220 };
    cpu_state.X = 1;
    cpu_state.pc = 0;
    cpu_state.cycle = 1;
    int signal_strength_total = 0;
    for (unsigned int i = 0; i < array_count(sample_points); i++) {
        int signal_strength = execute(&cpu_state, sample_points[i]);
        printf("Signal strength at cycle %4d: %6d\n", sample_points[i], signal_strength);
        signal_strength_total += signal_strength;
    }
    printf("Signal strength total: %d\n", signal_strength_total);

    return EXIT_SUCCESS;
}

