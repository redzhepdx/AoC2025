#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX

#include "../header/nob.h"
#include "../header/std_ds.h"

#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    a < _b ? _a : _b; })

typedef struct {
    char** items;
    size_t capacity;
    size_t count;
} InputData;

#define LINE_MAX_LEN 100000
typedef struct {
    char** items;
    size_t count;
    size_t capacity;
} Strings;

static inline uint64_t ns_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

static char* strdup_s(const char* s) {
    size_t len = strlen(s) + 1;
    char* p = malloc(len);
    if (!p) return NULL;
    memcpy(p, s, len);
    return p;
}

void split(Strings* out, const char* text, const char* delimiter) {
    char* copy = strdup_s(text);
    if (!copy) {
        fprintf(stderr, "Failed to allocate memory for copy\n");
        return;
    }

    char* token = strtok(copy, delimiter);
    while (token != NULL) {
        char* tok_copy = strdup_s(token);
        if (!tok_copy) {
            fprintf(stderr, "Failed to allocate memory for token\n");
            break;  // stop adding more; keep what we have
        }
        da_append(out, tok_copy);
        token = strtok(NULL, delimiter);
    }

    free(copy);
}

void remove_spaces(char* str) {
    char* read = str;
    char* write = str;

    while (*read) {
        if (*read != ' ') {  // keep only non-space characters
            *write = *read;
            write++;
        }
        read++;
    }
    *write = '\0';  // null-terminate the cleaned string
}

InputData* copy_input_data(const InputData* src) {
    InputData* dest = (InputData*)malloc(sizeof(InputData));

    // Copy metadata
    dest->capacity = src->capacity;
    dest->count = src->count;

    // Allocate new array of char*
    dest->items = malloc(dest->capacity * sizeof(char*));
    if (!dest->items) {
        dest->capacity = dest->count = 0;
        return dest;  // handle alloc failure however you like
    }

    // Deep-copy each string
    for (size_t i = 0; i < src->count; i++) {
        dest->items[i] = strdup(src->items[i]);  // or manual malloc/copy
        if (!dest->items[i]) {
            // Clean up partial allocation on failure
            for (size_t j = 0; j < i; j++) free(dest->items[j]);
            free(dest->items);
            dest->items = NULL;
            dest->capacity = dest->count = 0;
            return dest;
        }
    }

    return dest;
}

InputData read_lines(const char* filename) {
    InputData input_data = {0};
    FILE* fp = fopen(filename, "r");

    if (!fp) return input_data;

    char buffer[LINE_MAX_LEN];

    while (fgets(buffer, LINE_MAX_LEN, fp)) {
        // Trim newline
        buffer[strcspn(buffer, "\n")] = '\0';

        da_append(&input_data, nob_temp_strdup(buffer));
    }

    fclose(fp);
    return input_data;
}

void split_into_chunks(InputData* out, const char* text, size_t chunk_size) {
    size_t len = strlen(text);

    for (size_t i = 0; i < len; i += chunk_size) {
        size_t remaining = len - i;
        size_t take = remaining < chunk_size ? remaining : chunk_size;

        char* chunk = malloc(take + 1);
        memcpy(chunk, text + i, take);
        chunk[take] = '\0';

        nob_da_append(out, chunk);
    }
}

char* extract_segment(const char* text, size_t start_index, size_t end_index) {
    if (start_index >= end_index)
        return "";

    size_t segment_size = end_index - start_index;

    char* segment = malloc(segment_size + 1);
    memcpy(segment, text + start_index, segment_size);
    segment[segment_size] = '\0';

    return segment;
}

size_t char_to_int(char x) {
    return (size_t)(x - '0');
}

char* concat(const char* a, const char* b) {
    size_t len = strlen(a) + strlen(b) + 1;
    char* res = malloc(len);
    snprintf(res, len, "%s%s", a, b);
    return res;
}

int cmp_size_t(const void* a, const void* b) {
    size_t x = *(const size_t*)a;
    size_t y = *(const size_t*)b;

    if (x > y) return -1;
    if (x < y) return 1;
    return 0;
}

typedef struct {
    char* key;
    Strings value;
} Graph;

typedef struct {
    char* key;
    bool value;
} Visited;

typedef struct {
    char* key;
    uint64_t value;
} DiscoveredPaths;

uint64_t dfs(const Graph* graph, char* next) {
    if (strcmp(next, "out") == 0) {
        return 1;
    }

    uint64_t total = 0;
    // printf("Next Node : %s\n", next);

    Strings outputs = shget(graph, next);
    for (size_t idx = 0; idx < outputs.count; ++idx) {
        total += dfs(graph, outputs.items[idx]);
    }
    return total;
}

uint64_t solve(const InputData* lines) {
    Graph* graph = NULL;

    char* start;

    for (size_t line_idx = 0; line_idx < lines->count; ++line_idx) {
        char* line = lines->items[line_idx];
        printf("%s\n", line);

        Strings out = {0};
        split(&out, line, " ");

        char* input = out.items[0];

        input = extract_segment(input, 0, strlen(input) - 1);

        if (strcmp(input, "you") == 0) {
            start = input;
        }

        Strings outputs = {0};
        for (size_t idx = 1; idx < out.count; ++idx) {
            da_append(&outputs, out.items[idx]);
        }

        shput(graph, input, outputs);
    }

    printf("-------------------------\n");
    uint64_t total_ways = dfs(graph, "you");
    return total_ways;
}

uint64_t dfs_v2(const Graph* graph, char* next, char* target, Visited** visited, DiscoveredPaths** discovered_paths) {
    /*
    DFS with a lot of book keeping. Fast enough. Open to improvements.
    */
    if (strcmp(next, target) == 0) {
        shdel(*visited, next);
        return 1;
    }

    int res = shgeti(graph, next);
    if (res == -1) {
        return 0;
    }

    // // Visited nodes? Do we need it?
    // if (shgeti(*visited, next) != -1) {
    //     return 0;
    // }

    // If we have found this node, we know that there is only discovered amount of ways.
    // Whatever we put inside of this list will never be visited it will help us to build the DP.
    if (shgeti(*discovered_paths, next) != -1) {
        return shget(*discovered_paths, next);
    }

    // Mark it as visited
    shput(*visited, next, true);

    Strings outputs = shget(graph, next);
    uint64_t total = 0;

    for (size_t idx = 0; idx < outputs.count; ++idx) {
        int res = shgeti(*visited, outputs.items[idx]);

        if (res == -1) {
            total += dfs_v2(graph, outputs.items[idx], target, visited, discovered_paths);
        }
    }

    // Release this node so it can be visited again
    shdel(*visited, next);

    // DP part of the solution, memorize the number of paths up to here
    shput(*discovered_paths, next, total);

    return total;
}

uint64_t solve_part_2(const InputData* lines) {
    Graph* graph = NULL;

    char* start;

    for (size_t line_idx = 0; line_idx < lines->count; ++line_idx) {
        char* line = lines->items[line_idx];
        printf("%s\n", line);

        Strings out = {0};
        split(&out, line, " ");

        char* input = out.items[0];

        input = extract_segment(input, 0, strlen(input) - 1);

        if (strcmp(input, "you") == 0) {
            start = input;
        }

        Strings outputs = {0};
        for (size_t idx = 1; idx < out.count; ++idx) {
            da_append(&outputs, out.items[idx]);
        }

        shput(graph, input, outputs);
    }

    Visited* visited = NULL;
    DiscoveredPaths* discovered_paths = NULL;

    // Learned this brilliant division trick from https://www.reddit.com/user/mine49er/
    // SVR->DAC->FFT->OUT + SVR->FFT->DAC->OUT
    printf("-------------------------\n");
    uint64_t total_ways_path_1 = dfs_v2(graph, "svr", "dac", &visited, &discovered_paths);
    shfree(visited);
    shfree(discovered_paths);
    printf("SVR -> DAC DONE\n");

    total_ways_path_1 *= dfs_v2(graph, "dac", "fft", &visited, &discovered_paths);
    shfree(visited);
    shfree(discovered_paths);
    printf("DAC -> FFT DONE\n");

    total_ways_path_1 *= dfs_v2(graph, "fft", "out", &visited, &discovered_paths);
    shfree(visited);
    shfree(discovered_paths);
    printf("FFT -> OUT DONE\n");

    uint64_t total_ways_path_2 = dfs_v2(graph, "svr", "fft", &visited, &discovered_paths);
    shfree(visited);
    shfree(discovered_paths);
    printf("SVR -> FFT DONE\n");

    total_ways_path_2 *= dfs_v2(graph, "fft", "dac", &visited, &discovered_paths);
    shfree(visited);
    shfree(discovered_paths);
    printf("FFT -> DAC DONE\n");

    total_ways_path_2 *= dfs_v2(graph, "dac", "out", &visited, &discovered_paths);
    shfree(visited);
    shfree(discovered_paths);
    printf("DAC -> OUT DONE\n");

    uint64_t total_ways = total_ways_path_1 + total_ways_path_2;

    return total_ways;
}

int main() {
    char* input_file = "inputs/q11_input.txt";

    InputData input_lines = read_lines(input_file);

    uint64_t password = solve(&input_lines);
    uint64_t password = solve_part_2(&input_lines);
    printf("Password : %" PRIu64 "\n", password);

    da_free(input_lines);

    return 0;
}