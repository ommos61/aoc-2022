
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

// Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output

int list_count = 0;
#define MAX_LISTS 500
struct list {
    char *data;
} lists[MAX_LISTS];

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

        if (strlen(line) != 0) {
            // parse the data
            char *list = malloc(strlen(line) + 1);
            strcpy(list, line);
            lists[list_count].data = list;
            list_count++;
            assert((list_count <= MAX_LISTS) && "lists storage is too small");
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
    int result = 255;
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
        result = 0;
    } else if (isdigit(*list1) && isdigit(*list2)) {
        // compare the numbers
        int num1 = get_number(&list1);
        int num2 = get_number(&list2);
        //if (debug) printf("  comparing numbers %d and %d.\n", num1, num2);
        if (num1 > num2) {
            // incorrect order, so bail out
            result = 1;
        } else if (num1 == num2) {
            // same numbers, so continue checking
            result = check_pair(list1, list2);
        } else {
            // right order
            result = -1;
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
        result = 1;
    } else if ((*list1 == ']') && ((*list2 == ',') || isdigit(*list2))) {
        // left list ran out of items
        result = -1;
    } else if ((*list1 == '[') && (*list2 == ']')) {
        // an item in list1, but not in list2
        result = 1;
    } else if ((*list1 == ']') && (*list2 == '[')) {
        // an item in list2, but not in list1
        result = -1;
    } else {
        printf("Unhandled situation for pairs '%s' and '%s'.\n", list1, list2);
        result = 0;
    }

    return result;
}

void print_lists(void) {
    for (int i = 0; i < list_count; i++) {
        printf("%4d: %s\n", i + 1, lists[i].data);
    }
}

int list_compare(const void *in1, const void *in2) {
    struct list *list1 = (struct list *)in1;
    struct list *list2 = (struct list *)in2;
    assert(list1->data != NULL);
    assert(list2->data != NULL);

    int result = check_pair(list1->data, list2->data);
    if (result == 255) {
        printf("The check_pair() returned 255.\n");
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
    printf("There are %d lists.\n", list_count);
    // add the dividers
#define DIVIDER1 "[[2]]"
#define DIVIDER2 "[[6]]"
    lists[list_count].data = malloc(strlen(DIVIDER1) + 1);
    strcpy(lists[list_count++].data, DIVIDER1);
    lists[list_count].data = malloc(strlen(DIVIDER2) + 1);
    strcpy(lists[list_count++].data, DIVIDER2);

    // implement algorithm
    // sort the lists
    qsort(lists, list_count, sizeof(struct list), list_compare);
    if (debug) print_lists();

    // find the dividers
    int div1 = -1, div2 = -1;
    for (int i = 0; i < list_count; i++) {
        if (strcmp(lists[i].data, DIVIDER1) == 0) div1 = i + 1;
        if (strcmp(lists[i].data, DIVIDER2) == 0) div2 = i + 1;
    }
    printf("The decoder key is %d.\n", div1 * div2);

    return EXIT_SUCCESS;
}

