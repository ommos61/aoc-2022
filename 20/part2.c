
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

#define DECRYPTION_KEY 811589153L
int number_count = 0;
struct number {
    int index;
    long number;
    struct number *next, *prev;
};
struct number *start = NULL;

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
        if (sscanf(line, "%d", &value) == 1) {
            // put the data in the global structure
            struct number *number = malloc(sizeof(struct number));
            number->index = number_count;
            number->number = value * DECRYPTION_KEY;
            if (start == NULL) {
                start = number;
                number->next = number;
                number->prev = number;
            } else {
                number->next = start;
                number->prev = start->prev;
                start->prev = number;
                number->prev->next = number;
            }
            number_count += 1;
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

void print_file(void) {
    struct number *current = start;
    int first = 1;
    for (int i = 0; i < number_count; i++) {
        printf("%s%ld", first ? "" : ", ", current->number);
        first = 0;
        current = current->next;
    }
    assert(current == start);
    printf("\n");

#if 0
    // Also in reverse
    current = start->prev;
    first = 1;
    for (int i = 0; i < number_count; i++) {
        printf("%s%ld", first ? "" : ", ", current->number);
        first = 0;
        current = current->prev;
    }
    assert(current == start->prev);
    printf("\n");
#endif
}

struct number *find_index(int index) {
    struct number *current = start;

    while (current->index != index) current = current->next;

    return current;
}

struct number *find_number(long number) {
    struct number *current = start;

    while (current->number != number) current = current->next;

    return current;
}

void decrypt(void) {
#if 0
    if (debug) printf("Original list:\n");
    if (debug) print_file();
#endif
    for (int i = 0; i < number_count; i++) {
        struct number *current = find_index(i);
        long number = current->number;
        //if (debug) printf("Moving %ld\n", number);

        // 0 doesn't move
        if (number == 0) continue;

        // remove the item
        current->prev->next = current->next;
        current->next->prev = current->prev;

        // determine the target location
        struct number *target = current;
        number = number % (number_count - 1);
        while (number != 0) {
            if (number > 0) {
                target = target->next;
                number -= 1;
            } else if (number < 0) {
                target = target->prev;
                number += 1;
            }
        }
        if (current->number < 0) target = target->prev;

        if (current == start) start = current->next;
        // insert after the target
        //if (debug) printf("Inserting after %ld\n", target->number);
        current->next = target->next;
        current->prev = target;
        target->next->prev = current;
        target->next = current;

#if 0
        if (debug) print_file();
#endif
    }
}

struct number *get_forward(struct number *pos, int count) {
    struct number *result = pos;
    for (int i = 0; i < count; i++) {
        result = result->next;
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
    printf("There are %d numbers in the encrypted file.\n", number_count);

    // implement algorithm
    if (debug) printf("Original list:\n");
    if (debug) print_file();
    for (int i = 0; i < 10; i++) {
        decrypt();
        if (debug) printf("After itteration %d:\n", i + 1);
        if (debug) print_file();
    }
    struct number *zero = find_number(0);
    assert(zero->number == 0);
    struct number *x = get_forward(zero, 1000);
    struct number *y = get_forward(x, 1000);
    struct number *z = get_forward(y, 1000);
    printf("The magic numbers are: (%ld, %ld, %ld)\n", x->number, y->number, z->number);
    printf("The sum of those numbers is: %ld\n", x->number + y->number + z->number);

    return EXIT_SUCCESS;
}

