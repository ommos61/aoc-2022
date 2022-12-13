
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output

int pair_count = 0;
#define MAX_PAIRS 500
struct pair {
    char *list1;
    char *list2;
} pairs[MAX_PAIRS];

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    int list_id = 0;
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (strlen(line) != 0) {
            // TODO: parse the data
            char *list = malloc(strlen(line) + 1);
            strcpy(list, line);
            if (list_id == 0) {
                pairs[pair_count].list1 = list;
                list_id = 1;
            } else if (list_id == 1) {
                pairs[pair_count].list2 = list;
                list_id = 0;
                pair_count++;
                assert((pair_count <= MAX_PAIRS) && "pairs storage is too small");
            } else {
                assert(0 && "Should not come here");
            }
        } else if (strlen(line) == 0) {
            // ignore
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

int get_number(char **pl) {
    int number = 0;
    char *list = *pl;

    while (isdigit(*list)) {
        number = number * 10 + (*list - '0');
        list++;
    }
    *pl = list;

    return number;
}

struct alloc_item {
    char *data;
    struct alloc_item *next;
} *alloc_list = NULL;
void free_allocations(void) {
    int free_count = 0;
    struct alloc_item *item = alloc_list;

    while (item != NULL) {
        struct alloc_item *tmp = item;
        item = item->next;

        free(tmp->data);
        free(tmp);
        free_count++;
    }
    alloc_list = NULL;
    if (debug) printf("Freed %d newly created temporary lists.\n", free_count);
}

char *make_new_list(int num, char *remainder) {
    // allocate new item
    struct alloc_item *item = malloc(sizeof(struct alloc_item));
    item->next = alloc_list;
    alloc_list = item;

    // calculate the size of the new item data
    char num_str[20];
    sprintf(num_str, "%d", num);
    int item_data_size = 1 + strlen(num_str) + 1 + strlen(remainder);
    item->data = malloc(item_data_size + 1);
    sprintf(item->data, "[%d]%s", num, remainder);

    return item->data;
}

int check_pair(char *list1, char *list2) {
    int result = 1;
    assert((list1 != NULL) && (list2 != NULL));

    if ((*list1 == '[') && (*list2 == '[')) {
        // compare the remainder
        result = check_pair(list1 + 1, list2 + 1);
    } else if ((*list1 == ']') && (*list2 == ']')) {
        // compare the remainder
        result = check_pair(list1 + 1, list2 + 1);
    } else if ((*list1 == ',') && (*list2 == ',')) {
        // compare the remainder
        result = check_pair(list1 + 1, list2 + 1);
    } else if ((*list1 == '\000') && (*list2 == '\000')) {
        // lists are in the correct order, because end reached
        result = 1;
    } else if (isdigit(*list1) && isdigit(*list2)) {
        // compare the numbers
        int num1 = get_number(&list1);
        int num2 = get_number(&list2);
        //if (debug) printf("  comparing numbers %d and %d.\n", num1, num2);
        if (num1 > num2) {
            // incorrect order, so bail out
            result = 0;
        } else if (num1 == num2) {
            // same numbers, so continue checking
            result = check_pair(list1, list2);
        } else {
            // right order
            result = 1;
        }
    } else if (isdigit(*list1) && (*list2 == '[')) {
        // compare digit and list
        int num = get_number(&list1);
        char *new_list = make_new_list(num, list1);
        result = check_pair(new_list, list2);
    } else if ((*list1 == '[') && isdigit(*list2)) {
        // compare list and digit
        int num = get_number(&list2);
        char *new_list = make_new_list(num, list2);
        result = check_pair(list1, new_list);
    } else if (((*list1 == ',') || isdigit(*list1)) && (*list2 == ']')) {
        // right list ran out of items
        result = 0;
    } else if ((*list1 == ']') && ((*list2 == ',') || isdigit(*list2))) {
        // left list ran out of items
        result = 1;
    } else if ((*list1 == '[') && (*list2 == ']')) {
        // an item in list1, but not in list2
        result = 0;
    } else if ((*list1 == ']') && (*list2 == '[')) {
        // an item in list2, but not in list1
        result = 1;
    } else {
        printf("Unhandled situation for pairs '%s' and '%s'.\n", list1, list2);
        result = 0;
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
    printf("There are %d list pairs.\n", pair_count);

    // implement algorithm
    int index_sum = 0;
    for (int i = 0; i < pair_count; i++) {
        if (debug) printf("----> Comparing pair %d:\n", i + 1);
        if (debug) printf(" 1: %s\n", pairs[i].list1);
        if (debug) printf(" 2: %s\n", pairs[i].list2);
        int correct_order = check_pair(pairs[i].list1, pairs[i].list2);
        if (correct_order) {
            printf("Pair %d is in the correct order.\n", i + 1);
            index_sum += i + 1;
        } else {
            printf("Pair %d is NOT in the correct order.\n", i + 1);
        }

        free_allocations();
    }
    printf("Sum of indices for correct pairs is %d.\n", index_sum);

    return EXIT_SUCCESS;
}

