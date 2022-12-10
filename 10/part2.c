
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

#define LINE_POS(x) ((x) % 40)
void draw_pixel(struct cpu_state *state, int pos) {
    char pixel = '.';
    assert(state != NULL);
    assert(pos >= 0);

    int sprite_pos = state->X;
    if ((LINE_POS(pos) >= sprite_pos - 1)  && (LINE_POS(pos) <= sprite_pos + 1)) {
        pixel = '#';
    }
    printf("%c", pixel);
    if ((pos + 1) % 40 == 0) printf("\n");
}

void execute_line(struct cpu_state *state, int end_cycle) {
    static int pos = 0;
    assert(state != NULL);
    assert(end_cycle >= 0);

    while (state->cycle <= end_cycle) {
        assert((state->pc < instruction_count) && "The program is not long enough");
        switch (instructions[state->pc].command) {
        case OP_ADDX:
            draw_pixel(state, pos++);
            state->cycle += 1;
            draw_pixel(state, pos++);
            state->cycle += 1;
            state->X += instructions[state->pc].value;
            break;
        case OP_NOOP:
            draw_pixel(state, pos++);
            state->cycle += 1;
            break;
        default:
            assert(0 && "Unknown instruction encountered");
            break;
        }
        state->pc += 1;
    }
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
    cpu_state.X = 1;
    cpu_state.pc = 0;
    cpu_state.cycle = 1;
    for (int line = 1; line <= 6; line++) {
        execute_line(&cpu_state, line * 40);
    }

    return EXIT_SUCCESS;
}

