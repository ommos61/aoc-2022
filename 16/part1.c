
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

int valve_count = 0;
#define MAX_VALVES 100
#define MAX_TUNNELS 10
struct valvelist;
struct valve {
    char *name;
    int pressure;
    int open;
    char *tunnellist;
    int tunnel_count;
    int tunnels[MAX_TUNNELS];
} valves[MAX_VALVES];

void add_valve(char *name, int pressure, char *tunnels) {
    valves[valve_count].name = malloc(strlen(name) + 1);
    strcpy(valves[valve_count].name, name);
    valves[valve_count].pressure = pressure;
    valves[valve_count].tunnellist = malloc(strlen(tunnels) + 1);
    strcpy(valves[valve_count].tunnellist, tunnels);
    valves[valve_count].tunnel_count = 0;

    valve_count += 1;
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

        char id1[3], idlist[LINE_LENGTH];
        int pressure = 0;
        if (sscanf(line, "Valve %s has flow rate=%d; tunnels lead to valves %[^\n]", id1, &pressure, idlist) == 3) {
            add_valve(id1, pressure, idlist);
        } else if (sscanf(line, "Valve %s has flow rate=%d; tunnel leads to valve %[^\n]", id1, &pressure, idlist) == 3) {
            add_valve(id1, pressure, idlist);
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

int find_valve_index(char *name) {
    int index = -1;

    for (int i = 0; i < valve_count; i++) {
        if (strncmp(valves[i].name, name, 2) == 0) {
            index = i;
            break;
        }
    }

    return index;
}

void build_tunnels(void) {
    for (int v = 0; v < valve_count; v++) {
        char *tunnels = valves[v].tunnellist;
        int tunnel_count = 0;
        //printf("Initial list: '%s'\n", tunnels);

        while (strlen(tunnels) != 0) {
            int index = find_valve_index(tunnels);
            assert((index != -1) && "Seems the valve was not found");
            valves[v].tunnels[tunnel_count] = index;
            tunnel_count += 1;
            assert(tunnel_count < MAX_TUNNELS);

            // skip to next valve/tunnel in list
            tunnels += 2;
            while ((*tunnels == ',') || isspace(*tunnels)) tunnels += 1;
            //printf("Remaining in list: '%s'\n", tunnels);
        }
        valves[v].tunnel_count = tunnel_count;
    }
}

void print_valves(void) {
    for (int v = 0; v < valve_count; v++) {
        printf("%s(%d) -> { ", valves[v].name, valves[v].pressure);
        for (int t = 0; t < valves[v].tunnel_count; t++) {
            printf("%s ", valves[valves[v].tunnels[t]].name);
        }
        printf("}\n");
    }
}

int distance_count = 0;
struct target {
    int valve;
    int dist;
    struct target *next;
};
struct distances {
    int source;
    struct target *target_list;
} distances[MAX_VALVES] = { 0 };

struct target *new_target(int valve, int dist) {
    struct target *new = malloc(sizeof(struct target));
    new->valve = valve;
    new->dist = dist;
    new->next = NULL;
    return new;
}

void print_distances(void) {
    printf("-------- distances ---------\n");
    for (int v = 0; v < distance_count; v++) {
        struct target *list = distances[v].target_list;
        printf("%s -> {", valves[distances[v].source].name);
        while (list != NULL) {
            printf(" %s: %d,", valves[list->valve].name, list->dist);
            list = list->next;
        }
        printf("}\n");
    }
}

void build_distances(void) {
    for (int v = 0; v < valve_count; v++) {
        if ((strcmp(valves[v].name, "AA") != 0) && (valves[v].pressure == 0)) {
            continue;
        }

        distances[distance_count].source = v;

        int visited[MAX_VALVES] = { 0 };
        visited[v] = 1;

        struct target *target_list = new_target(v, 0);
        target_list->next = new_target(find_valve_index("AA"), 0);
        struct target *queue = new_target(v, 0);
        while (queue != NULL) {
            // get item from the front of the queue
            struct target *cur = queue;
            queue = queue->next;
            for (int nb = 0; nb < valves[cur->valve].tunnel_count; nb++) {
                int nbv = valves[cur->valve].tunnels[nb];
                if (visited[nbv]) continue;
                visited[nbv] = 1;
                if (valves[nbv].pressure != 0) {
                    struct target *new = new_target(nbv, cur->dist + 1);
                    new->next = target_list;
                    target_list = new;
                }
                // add to the end of the    queue
                struct target **pos = &queue;
                while (*pos != NULL) pos = &((*pos)->next);
                *pos = new_target(nbv, cur->dist + 1);
            }

            free(cur);
        }

        // remove the target to itself and to AA, i.e. the ones with dist 0
#if 1
        struct target **p = &target_list;
        while (*p != NULL) {
            while ((*p != NULL) && ((*p)->dist == 0)) {
                *p = (*p)->next;
            }
            if (*p != NULL) p = &((*p)->next);
        }
#endif
        distances[distance_count].target_list = target_list;
        distance_count += 1;
        //print_distances();
    }
}

struct cache_item {
    int valve;
    int time;
    long openmask;
    int total_pressure;
    struct cache_item *next;
};
struct cache_item *cache = NULL;

struct cache_item *new_cache_item(int v, int t, long m, int p) {
    struct cache_item *new = malloc(sizeof(struct cache_item));
    new->valve = v;
    new->time = t;
    new->openmask = m;
    new->total_pressure = p;
    new->next = NULL;
    return new;
}

struct cache_item *find_item(int v, int t, long m) {
    struct cache_item *item = cache;
    while (item != NULL) {
        if ((item->valve == v) && (item->time == t) && (item->openmask == m)) {
            break;
        }
        item = item->next;
    }
    return item;
}

int dfs(int valve, int time, long openmask) {
    int maxval = 0;

    if (debug) printf("dfs(%s, %d, %lx)\n", valves[valve].name, time, openmask);
    struct cache_item *item = find_item(valve, time, openmask);
    if (item != NULL) return item->total_pressure;

    struct target *target = NULL;
    for (int i = 0; i < distance_count; i++) {
        if (distances[i].source == valve) {
            target = distances[i].target_list;
            break;
        }
    }
    assert(target != NULL);

    while (target != NULL) {
        long bit = 1 << target->valve;
        if ((openmask & bit) == 0) {
            int remtime = time - target->dist - 1;
            if (remtime > 0) {

                maxval = MAX(
                    maxval,
                    dfs(target->valve, remtime, openmask | bit)
                        + remtime * valves[target->valve].pressure);
            }
        }
        target = target->next;
    }

    item = new_cache_item(valve, time, openmask, maxval);
    item->next = cache;
    cache = item;
    return maxval;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %d valves.\n", valve_count);

    // implement algorithm
    build_tunnels();
    print_valves();
    build_distances();
    print_distances();
    int total = dfs(find_valve_index("AA"), 30, 0);
    printf("Total pressure is %d.\n", total);
    if (strcmp(fname, "input.txt") != 0) {
        printf("The example answer should be 1651.\n");
    }

    return EXIT_SUCCESS;
}

