
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

// The data structure for the directory tree
struct entry {
    char *name;
    int size;               // if size is -1 then this is a directory
    int size_recursive;     // recursive size of a directory
    struct entry *first;    // 'first' is the first entry in the subdirectory
    struct entry *next;
};
struct entry root = { 0 };
struct entry *current_dir = &root;

int current_stack_depth = 0;
#define MAX_STACK_DEPTH 50
struct entry *dir_stack[MAX_STACK_DEPTH];

// Function forward declarations
void print_tree(struct entry *dir, int indent);

struct entry *find_dir(struct entry *current_dir, char *name) {
    struct entry *dir = NULL;

    struct entry *e = current_dir->first;
    while (e != NULL) {
        if (strcmp(e->name, name) == 0) {
            dir = e;
            assert((dir->size == -1) && "the found name doesn't seem to be a directory");
            break;
        }
        e = e->next;
    }

    return dir;
}

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    root.name = "/";
    root.size = -1;
    root.first = NULL;
    root.next = NULL;
    current_dir = &root;
    dir_stack[0] = &root;

    int line_count = 0;
    char line[LINE_LENGTH];
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;


        char new_name[1024];
        int new_size;
        if (strncmp(line, "$ cd /", 6) == 0) {
            current_dir = &root;
        } else if (strncmp(line, "$ cd ..", 7) == 0) {
            current_stack_depth--;
            current_dir = dir_stack[current_stack_depth];
        } else if (strncmp(line, "$ cd ", 4) == 0) {
            char *dir_name = line + 5;
            struct entry *new_dir = find_dir(current_dir, dir_name);
            if (new_dir != NULL) {
                current_stack_depth++;
                dir_stack[current_stack_depth] = new_dir;
                current_dir = new_dir;
            } else {
                fprintf(stderr, "Couldn't find '%s' as subdirectory in '%s'\n", dir_name, current_dir->name);
            }
        } else if (strncmp(line, "$ ls", 3) == 0) {
            // ignore
        } else if (sscanf(line, "dir %s", new_name) == 1) {
            // TODO: maybe check first if an entry with that name already exists
            //printf("Creating directory '%s'\n", new_name);
            struct entry *new_dir = malloc(sizeof(struct entry));
            new_dir->name = malloc(strlen(new_name) + 1);
            strcpy(new_dir->name, new_name);
            new_dir->size = -1;
            new_dir->size_recursive = 0;

            new_dir->next = current_dir->first;
            current_dir->first = new_dir;
            new_dir->first = NULL;
        } else if (sscanf(line, "%d %s", &new_size, new_name) == 2) {
            // TODO: maybe check first if an entry with that name already exists
            //printf("Creating file '%s' with size %d\n", new_name, new_size);
            struct entry *new_file = malloc(sizeof(struct entry));
            new_file->name = malloc(strlen(new_name) + 1);
            strcpy(new_file->name, new_name);
            new_file->size = new_size;
            new_file->size_recursive = new_size;

            new_file->next = current_dir->first;
            current_dir->first = new_file;
            new_file->first = NULL;
        } else if (errno != 0) {
            perror("sscanf");
        } else {
            fprintf(stderr, "Unexpected input format '%s'.\n", line);
        }
        // printf("------------------------------\n");
        // print_tree(&root, 0);

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

void print_tree(struct entry *dir, int indent) {
    struct entry *e = dir;

    while (e != NULL) {
        for (int i = 0; i < indent; i++) printf("  ");
        printf("%s", e->name);
        if (e->size == -1) printf(" [%d]", e->size_recursive);
        printf("\n");
        if (e->size == -1) {
            print_tree(e->first, indent + 1);
        }

        e = e->next;
    }
}

void calculate_recursive(struct entry *entry) {
    int total = 0;
    assert(entry != NULL);

    if (entry->size == -1) {
        struct entry *e = entry->first;
        while (e != NULL) {
            if (e->size == -1) {
                calculate_recursive(e);
                total += e->size_recursive;
            } else {
                total += e->size;
            }

            e = e->next;
        }
        entry->size_recursive = total;
    }

    return;
}

int get_total_dirs_max(struct entry *dir, int max_size) {
    int total = 0;
    assert((dir->size == -1) && "can only be done for directories");

    struct entry *e = dir->first;
    while (e != NULL) {
        if (e->size == -1) {
            if (e->size_recursive <= max_size) {
                total += e->size_recursive;
            }
            total += get_total_dirs_max(e, max_size);
        }
        e = e->next;
    }

    return total;
}

struct entry *find_smallest(struct entry *dir, int minimum) {
    struct entry *smallest_dir = NULL;
    assert((minimum > 0) && "minimum must be bigger than 0");

    struct entry *e = dir->first;
    while (e != NULL) {
        if (e->size == -1) {
            struct entry *smallest = find_smallest(e, minimum);
            if (smallest != NULL) {
                if ((smallest_dir == NULL) || (smallest_dir->size_recursive > smallest->size_recursive)) {
                    smallest_dir = smallest;
                }
            } else if (e->size_recursive > minimum) {
                if ((smallest_dir == NULL) || (smallest_dir->size_recursive > e->size_recursive)) {
                    smallest_dir = e;
                }
            }
        }
        
        e = e->next;
    }

    return smallest_dir;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // TODO: implement algorithm
    calculate_recursive(&root);
    // print_tree(&root, 0);
    //int total = get_total_dirs_max(&root, 100000);
    int filesystem_size = 70000000;
    int free_size_needed = 30000000;
    printf("FS size is   %8d\n", filesystem_size);
    printf("Free needed  %8d\n", free_size_needed);
    printf("Total / is   %8d\n", root.size_recursive);
    int free_space = filesystem_size - root.size_recursive;
    printf("Free space   %8d\n", free_space);
    int needed = free_size_needed - free_space;
    printf("Need to free %8d\n", needed);

    struct entry *smallest_dir = find_smallest(&root, needed);
    if (smallest_dir != NULL) {
        printf("Remove '%s' with size %d\n", smallest_dir->name, smallest_dir->size_recursive);
    } else {
        printf("Couldn't find a directory that is big enough to remove\n");
    }

    return EXIT_SUCCESS;
}

