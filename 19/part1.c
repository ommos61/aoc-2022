
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 1; // 0 = no debug output, 1 = some extra debug output
#define OPTIMIZE 1

int blueprint_count = 0;
#define MAX_BLUEPRINTS 50
#define ID_ORE      0
#define ID_CLAY     1
#define ID_OBSIDIAN 2
#define ID_GEODE    3
#define ID_SIZE     4
struct blueprint {
    int id;
    int cost[ID_SIZE][ID_SIZE];
} blueprints[MAX_BLUEPRINTS];
int maxval[ID_SIZE] = { 0 };

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

        int v1, v2, v3, v4, v5, v6, v7;
        if (sscanf(line, "Blueprint %d: Each ore robot costs %d ore. Each clay robot costs %d ore. Each obsidian robot costs %d ore and %d clay. Each geode robot costs %d ore and %d obsidian.", &v1, &v2, &v3, &v4, &v5, &v6, &v7) == 7) {
            // parse the data
            memset(blueprints + blueprint_count, 0, sizeof(struct blueprint));
            blueprints[blueprint_count].id = v1;
            blueprints[blueprint_count].cost[ID_ORE][ID_ORE] = v2;
            blueprints[blueprint_count].cost[ID_CLAY][ID_ORE] = v3;
            blueprints[blueprint_count].cost[ID_OBSIDIAN][ID_ORE] = v4;
            blueprints[blueprint_count].cost[ID_OBSIDIAN][ID_CLAY] = v5;
            blueprints[blueprint_count].cost[ID_GEODE][ID_ORE] = v6;
            blueprints[blueprint_count].cost[ID_GEODE][ID_OBSIDIAN] = v7;
            blueprint_count++;
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

void print_blueprint(struct blueprint bp) {
    printf("%2d: ", bp.id);
    for (int i = 0; i < ID_SIZE; i++) {
        printf("(");
        for (int j = 0; j < ID_SIZE - 1; j++) {
            printf("%d,", bp.cost[i][j]);
        }
        printf(") ");
    }
    printf("\n");
}
             
struct state {
    int time;
    int bag[ID_SIZE];
    int robots[ID_SIZE];
};

void print_state(struct state *s) {
    printf("t=%d,r=(", s->time);
    for (int i = 0; i < ID_SIZE; i++) {
        printf("%d,", s->robots[i]);
    }
    printf("),b=(");
    for (int i = 0; i < ID_SIZE; i++) {
        printf("%d,", s->bag[i]);
    }
    printf(")\n");
}

struct state *copy_state(struct state *in) {
    struct state *copy = malloc(sizeof(struct state));
    memcpy(copy, in, sizeof(struct state));
    return copy;
}

int cieldiv(int a, int d) {
    int result = 0;
    if (a > 0) {
        result = a / d;
        if (a % d != 0) result += 1;
    }
    return result;
}

int time_before_build_robot(struct state *in, struct blueprint bp, int robot) {
    int time = 0;
    // no need to check for ore robots, because there is always at least one
    if (robot == ID_ORE) {
        time = cieldiv(bp.cost[ID_ORE][ID_ORE] - in->bag[ID_ORE], in->robots[ID_ORE]);
    } else if (robot == ID_CLAY) {
        time = cieldiv(bp.cost[ID_CLAY][ID_ORE] - in->bag[ID_ORE], in->robots[ID_ORE]);
    } else if ((robot == ID_OBSIDIAN) && (in->robots[ID_CLAY] != 0)) {
        int t1 = cieldiv(bp.cost[ID_OBSIDIAN][ID_ORE] - in->bag[ID_ORE], in->robots[ID_ORE]);
        int t2 = cieldiv(bp.cost[ID_OBSIDIAN][ID_CLAY] - in->bag[ID_CLAY], in->robots[ID_CLAY]);
        time = MAX(t1, t2);
    } else if ((robot == ID_GEODE) && (in->robots[ID_OBSIDIAN] != 0)) {
        int t1 = cieldiv(bp.cost[ID_GEODE][ID_ORE] - in->bag[ID_ORE], in->robots[ID_ORE]);
        int t2 = cieldiv(bp.cost[ID_GEODE][ID_OBSIDIAN] - in->bag[ID_OBSIDIAN], in->robots[ID_OBSIDIAN]);
        time = MAX(t1, t2);
    } else {
        time = -1;
    }
    return time;
}

int dfs(struct state *in, struct blueprint bp) {
    int max_geodes = 0;
    //printf("Time: %d\n", in.time);
    if (in->time == 0) {
        max_geodes = in->bag[ID_GEODE];
    } else {
        // Simulate the process
        // do not build any robot
        max_geodes = in->bag[ID_GEODE] + in->time * in->robots[ID_GEODE];

        // for each robot to build:
        //  - wait until enough material is available
        //  - build the robot
        //  - handle the remaining time
        for (int id = 0; id < ID_SIZE; id++) {
#if OPTIMIZE
            if ((id != ID_GEODE) && (in->robots[id] >= maxval[id])) {
                continue;
            }
#endif
            int wait = time_before_build_robot(in, bp, id);
            if (wait == -1) {
                // this robot can never be build with the current set of robots
                continue;
            } else if (in->time > wait + 1) {
                assert(wait >= 0);
                wait += 1;  // take into account the time to build the robot
                struct state *next = copy_state(in);

                // add the materials for the wait and subtract the ones for the build robot
                for (int matid = 0; matid < ID_SIZE; matid++) {
                    next->bag[matid] +=
                        (wait * in->robots[matid] - bp.cost[id][matid]);
                    if (next->bag[matid] < in->robots[matid]) {
                        print_state(in);
                        printf("build=%d,wait=%d\n", id, wait - 1);
                        print_state(next);
                    }
                    assert(next->bag[matid] >= in->robots[matid]);
                }

                // build the robot
                next->robots[id] += 1;

                // update the time
                next->time -= wait;

                // determine what happens next
                int new_geodes = dfs(next, bp);
                max_geodes = MAX(max_geodes, new_geodes);
                free(next);
            }
        }
    }

    return max_geodes;
}

int get_max_geodes(int blueprint_index, int time) {
    struct blueprint bp = blueprints[blueprint_index];
    if (debug) print_blueprint(bp);
    struct state state = { 0 };
    state.robots[ID_ORE] = 1;
    state.time = time;

    maxval[ID_ORE] = bp.cost[ID_ORE][ID_ORE];
    maxval[ID_ORE] = MAX(maxval[ID_ORE], bp.cost[ID_CLAY][ID_ORE]);
    maxval[ID_ORE] = MAX(maxval[ID_ORE], bp.cost[ID_OBSIDIAN][ID_ORE]);
    maxval[ID_ORE] = MAX(maxval[ID_ORE], bp.cost[ID_GEODE][ID_ORE]);
    maxval[ID_CLAY] = bp.cost[ID_OBSIDIAN][ID_CLAY];
    maxval[ID_OBSIDIAN] = bp.cost[ID_GEODE][ID_OBSIDIAN];
    if (debug) printf("max: %d, %d, %d\n", maxval[0], maxval[1], maxval[2]);

    clock_t start, end;
    start = clock();
    int geodes = dfs(&state, bp);
    end = clock();
    if (debug)
        printf("  dfs() took %.2f seconds.\n",
               ((double)(end - start)) / CLOCKS_PER_SEC);

    return geodes;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %d blueprints.\n", blueprint_count);

    // implement algorithm
#define MINUTES_TO_USE 24
    int total_quality_level = 0;
    for (int b = 0; b < blueprint_count; b++) {
        int geodes = get_max_geodes(b, MINUTES_TO_USE);
        if (debug) printf("Blueprint %d: %d geodes.\n", blueprints[b].id, geodes);
        total_quality_level += blueprints[b].id * geodes;
    }
    printf("Total quality level is %d.\n", total_quality_level);

    return EXIT_SUCCESS;
}

