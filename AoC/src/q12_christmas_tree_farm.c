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

typedef struct {
    uint8_t* items;
    size_t count;
    size_t capacity;
} Gifts;

uint64_t solve(const InputData* lines) {
    // Disgusting hueristic approach works for general use cases for AoC does it solve the real problem? NO! Is real problem easy? FUCK NO! It is np-hard.
    Gifts gifts = {0};
    for (size_t idx = 0; idx < 30; idx += 5) {
        uint8_t total_filled_tiles = 0;
        for (size_t gift_offset = 1; gift_offset <= 3; ++gift_offset) {
            char* gift_line = lines->items[idx + gift_offset];
            printf("%s\n", gift_line);
            for (size_t tid = 0; tid < strlen(gift_line); ++gift_line) {
                if (gift_line[tid] == '#') {
                    total_filled_tiles++;
                }
            }
        }
        da_append(&gifts, total_filled_tiles);
    }

    uint64_t valid_map_count = 0;

    for (size_t idx = 30; idx < lines->count; ++idx) {
        char* line = lines->items[idx];

        Strings out = {0};
        split(&out, line, " ");

        char* grid_size = out.items[0];
        char* new_grid_size = extract_segment(grid_size, 0, strlen(grid_size) - 1);

        Strings grid_size_out = {0};
        split(&grid_size_out, new_grid_size, "x");

        uint16_t row = (uint16_t)strtoll(grid_size_out.items[0], NULL, 10);
        uint16_t col = (uint16_t)strtoll(grid_size_out.items[1], NULL, 10);

        uint32_t total_occupied_area = 0;
        for (size_t req_idx = 1; req_idx < out.count; ++req_idx) {
            uint16_t req_count = (uint16_t)strtoll(out.items[req_idx], NULL, 10);
            printf("%d ", req_count);
            if (req_count > 0) {
                total_occupied_area += req_count * gifts.items[req_idx - 1];
            }
        }
        printf("\n");

        if (total_occupied_area < (row * col)) {
            printf("Valid Total Occupied area : %lu \n", total_occupied_area);
            valid_map_count++;
        }

        da_free(grid_size_out);
    }

    da_free(gifts);

    return valid_map_count;
}

uint64_t solve_part_2(const InputData* lines) {
}

int main() {
    char* input_file = "inputs/q12_input.txt";

    InputData input_lines = read_lines(input_file);

    uint64_t password = solve(&input_lines);
    uint64_t password = solve_part_2(&input_lines);
    printf("Password : %" PRIu64 "\n", password);
    
    da_free(input_lines);
    return 0;
}