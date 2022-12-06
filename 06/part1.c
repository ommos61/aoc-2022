
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

int stream_count = 0;
#define MAX_STREAMS 10
char *streams[MAX_STREAMS];
#define MAX_LINE_LENGTH 10000

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        char *stream = malloc(MAX_LINE_LENGTH);
        if (sscanf(line, "%s", stream) == 1) {
            // TODO: parse the data
            streams[stream_count] = stream;
            stream_count++;
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

int is_start_of_packet(char *s, int length) {
    int result = 1; // False

    //printf("%d: %4s\n", length, s);
    for (int i = 0; i < length; i++) {
        for (int j = i + 1; j < length; j++) {
            if (s[i] == s[j]) {
                result = 0;
                break;
            }
        }
        if (result == 0) break;
    }

    return result;
}

int find_marker(char *stream) {
    int result = 0;

    // printf("Stream: '%s'\n", stream);
    for (unsigned int i = 3; i < strlen(stream); i++) {
        if (is_start_of_packet(stream + (i - 3), 4)) {
            result = i;
            break;
        }
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

    // TODO: implement algorithm
    printf("Streams: %d\n", stream_count);
    for (int i = 0; i < stream_count; i++) {
        int marker = find_marker(streams[i]);
        if (marker != 0) {
            printf("Stream %d start-of-packet is at %d\n", i + 1, marker + 1);
        }
    }

    return EXIT_SUCCESS;
}

