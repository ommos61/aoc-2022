
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
int debug = 0; // 0 = no debug output, 1 = some extra debug output

// Monkey data structure
int monkey_count = 0;
#define MAX_MONKEYS 20
#define OP_ADD 1
#define OP_MUL 2
#define OP_SQR 3
struct item {
    int worry_level;
    struct item *next;
};
struct monkey {
    struct item *items;
    int inspect_op;
    int inspect_val;
    int test_divisor;
    int monkey_true;
    int monkey_false;
    int inspect_count;
} monkeys[MAX_MONKEYS];

void monkey_append_item(struct monkey *monkey, struct item *item) {
    if (monkey->items == NULL) {
        monkey->items = item;
    } else {
        struct item **loc = &(monkey->items);
        while (*loc != NULL) {
            loc = &((*loc)->next);
        }
        *loc = item;
    }
}

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

#define STARTING_ITEMS "  Starting items: "
#define OPERATION      "  Operation: new = old "
#define OP_ADD_STR     "+ %d"
#define OP_MUL_STR     "* %d"
#define OP_SQR_STR     "* old"
#define TEST           "  Test: divisible by %d"
#define RESULT_TRUE    "    If true: throw to monkey %d"
#define RESULT_FALSE   "    If false: throw to monkey %d"
        int value = 0;
        if (sscanf(line, "Monkey %d:", &value) == 1) {
            // the monkey number
            monkey_count = value + 1;
            monkeys[monkey_count - 1].items = NULL;
            monkeys[monkey_count - 1].inspect_count = 0;
        } else if (strncmp(line, STARTING_ITEMS, strlen(STARTING_ITEMS)) == 0) {
            // loop over the item list
            char *items = line + strlen(STARTING_ITEMS);
            while (*items != '\000') {
                //if (debug) printf("monkey %d items: %s\n", monkey_count - 1, items);
                if (sscanf(items, "%d", &value) == 1) {
                    struct item *item = malloc(sizeof(struct item));
                    item->worry_level = value;
                    item->next = NULL;
                    monkey_append_item(monkeys + monkey_count - 1, item);
                } else {
                }
                while ((*items >= '0') && (*items <= '9')) {
                    items++;
                }
                if (strncmp(items, ", ", 2) == 0) {
                    items += 2;
                }
            }
        } else if (strncmp(line, OPERATION, strlen(OPERATION)) == 0) {
            // parse the operation
            char *op = line + strlen(OPERATION);
            if (strcmp(op, OP_SQR_STR) == 0) {
                monkeys[monkey_count - 1].inspect_op = OP_SQR;
            } else if (sscanf(op, OP_ADD_STR, &value) == 1) {
                monkeys[monkey_count - 1].inspect_op = OP_ADD;
                monkeys[monkey_count - 1].inspect_val = value;
            } else if (sscanf(op, OP_MUL_STR, &value) == 1) {
                monkeys[monkey_count - 1].inspect_op = OP_MUL;
                monkeys[monkey_count - 1].inspect_val = value;
            } else {
                printf("Unknown operation '%s' in inspect operation", op);
            }
        } else if (sscanf(line, TEST, &value) == 1) {
            // save the divisable number
            monkeys[monkey_count - 1].test_divisor = value;
        } else if (sscanf(line, RESULT_TRUE, &value) == 1) {
            // save the true result monkey
            monkeys[monkey_count - 1].monkey_true = value;
        } else if (sscanf(line, RESULT_FALSE, &value) == 1) {
            // save the false result monkey
            monkeys[monkey_count - 1].monkey_false = value;
        } else if (strlen(line) == 0) {
            // skip
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

void print_monkey(int index) {
    printf("Monkey %d\n", index);
    printf(STARTING_ITEMS);
    struct item *item = monkeys[index].items;
    while (item != NULL) {
        printf("%d ", item->worry_level);
        item = item->next;
    }
    printf("\n");
    printf(OPERATION);
    switch (monkeys[index].inspect_op) {
    case OP_SQR:
        printf("* old\n");
        break;
    case OP_ADD:
        printf("+ %d\n", monkeys[index].inspect_val);
        break;
    case OP_MUL:
        printf("* %d\n", monkeys[index].inspect_val);
        break;
    default:
        printf("<!!! Unknown operation !!!>\n");
        break;
    }
    printf(TEST "\n", monkeys[index].test_divisor);
    printf(RESULT_TRUE "\n", monkeys[index].monkey_true);
    printf(RESULT_FALSE "\n", monkeys[index].monkey_false);
}

void print_monkey_items(int index) {
    printf("Monkey %d: ", index);
    struct item *item = monkeys[index].items;
    while (item != NULL) {
        printf("%d ", item->worry_level);
        item = item->next;
    }
    printf("\n");
}

void perform_monkey(int index) {
    assert(index >= 0);

    struct item *cur_item = monkeys[index].items;
    monkeys[index].items = NULL;
    while (cur_item != NULL) {
        monkeys[index].inspect_count++;
        switch (monkeys[index].inspect_op) {
        case OP_SQR:
            cur_item->worry_level *= cur_item->worry_level;
            break;
        case OP_ADD:
            cur_item->worry_level += monkeys[index].inspect_val;
            break;
        case OP_MUL:
            cur_item->worry_level *= monkeys[index].inspect_val;
            break;
        }
        cur_item->worry_level /= 3;
        struct item *next_item = cur_item->next;
        cur_item->next = NULL;
        if ((cur_item->worry_level % monkeys[index].test_divisor) == 0) {
            monkey_append_item(monkeys + monkeys[index].monkey_true, cur_item);
        } else {
            monkey_append_item(monkeys + monkeys[index].monkey_false, cur_item);
        }

        cur_item = next_item;
    }
}

void perform_round(void) {
    for (int i = 0; i < monkey_count; i++) {
        perform_monkey(i);
    }
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("Monkey count: %d\n", monkey_count);

    // TODO: implement algorithm
#define ROUND_COUNT 20
    for (int round = 1; round <= ROUND_COUNT; round++) {
        perform_round();
        if(debug) printf("---> after round %d\n", round);
        for (int i = 0; i < monkey_count; i++) {
            if (debug) print_monkey_items(i);
        }
    }
    printf("Inspection counts:\n");
    int max1 = -1, max2 = -1;
    for (int i = 0; i < monkey_count; i++) {
        int count = monkeys[i].inspect_count;
        printf("Monkey %d inspected %d items.\n", i, count);
        if ((count > max1) && (count > max2)) {
            if (max1 > max2) {
                max2 = count;
            } else {
                max1 = count;
            }
        } else if (count > max1) {
            max1 = count;
        } else if (count > max2) {
            max2 = count;
        }
    }
    printf("Level of monkey business is (%d * %d) = %d\n", max1, max2, max1 * max2);

    return EXIT_SUCCESS;
}

