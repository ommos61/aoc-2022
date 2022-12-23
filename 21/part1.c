
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

int monkey_count = 0;
#define MAX_MONKEYS 3000
#define TYPE_VALUE      1
#define TYPE_PLUS       2
#define TYPE_MINUS      3
#define TYPE_MULTIPLY   4
#define TYPE_DIVIDE     5
#define TYPE_CALCULATED 0
struct monkey {
    char name[5];
    int type;
    long value;
    char left[5], right[5];
} monkeys[MAX_MONKEYS];

// TODO: Global data information
int debug = 1; // 0 = no debug output, 1 = some extra debug output

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
        char name[5], name1[5], name2[5], op;
        if (sscanf(line, "%[^:]: %d", name, &value) == 2) {
            strcpy(monkeys[monkey_count].name, name);
            monkeys[monkey_count].type = TYPE_VALUE;
            monkeys[monkey_count].value = value;
            monkey_count += 1;
        } else if (sscanf(line, "%[^:]: %[^ ] %c %[^ ]", name, name1, &op, name2) == 4) {
            strcpy(monkeys[monkey_count].name, name);
            switch (op) {
            case '+':
                monkeys[monkey_count].type = TYPE_PLUS;
                break;
            case '-':
                monkeys[monkey_count].type = TYPE_MINUS;
                break;
            case '*':
                monkeys[monkey_count].type = TYPE_MULTIPLY;
                break;
            case '/':
                monkeys[monkey_count].type = TYPE_DIVIDE;
                break;
            default:
                assert(0 && "Unknown operation detected");
                break;
            }
            strcpy(monkeys[monkey_count].left, name1);
            strcpy(monkeys[monkey_count].right, name2);
            monkey_count += 1;
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

struct monkey *find_monkey(char *name) {
    struct monkey *found = NULL;
    for (int m = 0; m < monkey_count; m++) {
        if (strcmp(monkeys[m].name, name) == 0) {
            found = monkeys + m;
            break;
        }
    }
    return found;
}

long calculate(struct monkey *monkey) {
    long value = 0;

    if ((monkey->type == TYPE_VALUE)  || (monkey->type == TYPE_CALCULATED)) {
        value = monkey->value;
    } else {
        long value1 = calculate(find_monkey(monkey->left));
        long value2 = calculate(find_monkey(monkey->right));
        switch (monkey->type) {
        case TYPE_PLUS:
            value = value1 + value2;
            break;
        case TYPE_MINUS:
            value = value1 - value2;
            break;
        case TYPE_MULTIPLY:
            value = value1 * value2;
            break;
        case TYPE_DIVIDE:
            value = value1 / value2;
            break;
        default:
            assert(0 && "Unknown operation encountered");
            break;
        }
        monkey->value = value;
        monkey->type = TYPE_CALCULATED;
    }

    return value;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %d monkeys.\n", monkey_count);

    // implement algorithm
    struct monkey *root = find_monkey("root");
    long number = calculate(root);

    printf("The root monkey yells out the number %ld.\n", number);

    return EXIT_SUCCESS;
}

