
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

int num_count = 0;
#define MAX_NUMBERS 100
char *numbers[MAX_NUMBERS];

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

        if (strlen(line) > 0) {
            // copy the data
            assert(num_count < MAX_NUMBERS);
            char *number = malloc(strlen(line) + 1);
            strcpy(number, line);
            numbers[num_count] = number;
            num_count += 1;
        } else {
            fprintf(stderr, "Unexpected input format '%s'.\n", line);
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

long get_value(char *snafu) {
    long val = 0;

    while (*snafu != '\000') {
        val *= 5;
        switch (*snafu) {
        case '2':
        case '1':
        case '0':
            val += (*snafu - '0');
            break;
        case '-':
            val -= 1;
            break;
        case '=':
            val -= 2;
            break;
        default:
            assert(0 && "illegal character in SNAFU number");
        }
        snafu += 1;
    }

    return val;
}

char *reverse(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char t = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = t;
    }
    return str;
}

char *get_snafu(long val) {
    assert(val > 0);
#define MAX_SNAFU 100
    static char snafu[MAX_SNAFU];
    char *p = snafu;

    while (val > 0) {
        int m = val % 5;
        val /= 5;
        switch (m) {
        case 1:
        case 2:
            *p++ = '0' + m;
            break;
        case 3:
            *p++ = '=';
            val += 1;
            break;
        case 4:
            *p++ = '-';
            val += 1;
            break;
        case 0:
            *p++ = '0';
            break;
        }
    }
    *p = '\000';
    reverse(snafu);
    return snafu;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %d SNAFU numbers.\n", num_count);

    // implement algorithm
    // 4890: 2=-1=0
    long sum = 0;
    for (int i = 0; i < num_count; i++) {
        long val = get_value(numbers[i]);
        if (debug) printf("%20s: %ld\n", numbers[i], val);
        sum += val;
    }
    printf("The sum is %ld.\n", sum);
    char *snafu = get_snafu(sum);
    printf("The SNAFU sum is %s, which translates back to %ld.\n", snafu, get_value(snafu));

#if 0
    printf("reverse test:\n");
    char *strings[] = {
        "0",
        "01",
        "012",
        "0123",
        "01234",
        "012345",
        "0123456",
    };
    for (unsigned int i = 0; i < array_count(strings); i++) {
        char s[50];
        strcpy(s, strings[i]);
        printf("reverse(\"%s\") = \"%s\"\n", strings[i], reverse(s));
    }
#endif

    return EXIT_SUCCESS;
}

