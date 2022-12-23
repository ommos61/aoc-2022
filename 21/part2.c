
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
#define NAME_ROOT "root"
#define NAME_HUMAN "humn"
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
int find_humans(struct monkey *tree) {
    int count = 0;

    if ((tree->type == TYPE_VALUE) && (strcmp(tree->name, NAME_HUMAN) == 0)) {
        count += 1;
    } else if (tree->type != TYPE_VALUE) {
        count += find_humans(find_monkey(tree->left));
        count += find_humans(find_monkey(tree->right));
    }

    return count;
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

long solve_tree(struct monkey *tree, long target) {
    // This tree always contains the human
    assert(tree != NULL);
    assert(target != 0);
    long yell = 0;

    //printf("Target is %ld.\n", target);
    if ((tree->type == TYPE_VALUE) && (strcmp(tree->name, NAME_HUMAN) == 0)) {
        yell = target;
    } else {
        // so the top node is not the human, that means it is aways an operation
        int count = find_humans(find_monkey(tree->left));
        long val = 0;
        struct monkey *human_tree;
        if (count > 0) {
            // the left is the subtree with the human, so the right is calculatable
            val = calculate(find_monkey(tree->right));
            human_tree = find_monkey(tree->left);
        } else {
            // the right is the subtree with the human, so the left is calculatable
            val = calculate(find_monkey(tree->left));
            human_tree = find_monkey(tree->right);
        }
        long new_target = 0;
        switch (tree->type) {
        case TYPE_PLUS:
            new_target = target - val;
            break;
        case TYPE_MINUS:
            if (count > 0) {
                new_target = target + val;
            } else {
                new_target = val - target;
            }
            break;
        case TYPE_MULTIPLY:
            new_target = target / val;
            break;
        case TYPE_DIVIDE:
            if (count > 0) {
                new_target = target * val;
            } else {
                new_target = val / target;
            }
            break;
        default:
            assert(0 && "should not reach here");
            break;
        }
        assert(new_target != 0);
        yell = solve_tree(human_tree, new_target);
    }

    assert(yell != 0);
    return yell;
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
    struct monkey *root = find_monkey(NAME_ROOT);
    struct monkey *human = find_monkey(NAME_HUMAN);
    assert(root->type != TYPE_VALUE);
    assert(human->type == TYPE_VALUE);
    int count_left = find_humans(find_monkey(root->left));
    int count_right = find_humans(find_monkey(root->right));
    
    printf("Left contains %d humans, right contains %d humans.\n", count_left, count_right);
    assert((count_left + count_right) == 1);

    long target_number = 0;
    struct monkey *human_tree = NULL;
    if (count_left == 0) {
        target_number = calculate(find_monkey(root->left));
        human_tree = find_monkey(root->right);
    } else {
        assert(count_right == 0);
        target_number = calculate(find_monkey(root->right));
        human_tree = find_monkey(root->left);
    }
    printf("The target for the human-tree is %ld.\n", target_number);
    long yell = solve_tree(human_tree, target_number);
    printf("The human should yell %ld.\n", yell);

    // verify the answer
    human->value = yell;
    //human->type = TYPE_VALUE;
    printf("The human tree calculation results in %ld.\n", calculate(human_tree));

    printf("The anwser was not -681461662.\n");
    return EXIT_SUCCESS;
}

