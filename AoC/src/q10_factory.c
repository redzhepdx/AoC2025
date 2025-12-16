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

typedef struct
{
    size_t* items;
    size_t count;
    size_t capacity;
} JoltageReqs;

typedef struct
{
    size_t* items;
    size_t count;
    size_t capacity;
} ButtonSemantics;

typedef struct {
    size_t light_diagram;
    size_t light_diagram_len;
    ButtonSemantics button_semantics;
    JoltageReqs joltage_requirements;
} Diagram;

typedef struct {
    Diagram* items;
    size_t count;
    size_t capacity;
} Diagrams;

size_t diagram_to_value(char* diagram_str) {
    size_t len = strlen(diagram_str);
    size_t byte_length = len - 2;
    size_t value = 0;
    // Skip first and last letters
    for (size_t i = 1; i < len - 1; ++i) {
        char light_value = diagram_str[i];
        if (light_value == '#') {
            value += (1 << (byte_length - i));
        }
    }
    return value;
}

size_t semantic_to_value(char* semantic_str, size_t diagram_len) {
    size_t len = strlen(semantic_str);

    char* new_semantic_str = extract_segment(semantic_str, 1, len - 1);

    len = strlen(new_semantic_str);
    Strings nums = {0};
    split(&nums, new_semantic_str, ",");
    size_t value = 0;

    // Skip first and last letters
    for (size_t i = 0; i < nums.count; ++i) {
        char* light_value = nums.items[i];
        size_t button_number = (size_t)strtoll(nums.items[i], NULL, 10);
        size_t diff = (diagram_len - button_number);
        size_t mask_value = (1 << diff);
        value += mask_value;
    }

    free(new_semantic_str);
    da_free(nums);

    return value;
}

size_t dfs(Diagram* d, size_t current_value, size_t target_value, size_t button_pressed, size_t last_index) {
    if (last_index > d->button_semantics.count) {
        return INT_FAST16_MAX;
    }

    if (current_value == target_value) {
        return button_pressed;
    }

    size_t button_semantic = d->button_semantics.items[last_index];
    size_t hit = dfs(d, current_value ^ button_semantic, target_value, button_pressed + 1, last_index + 1);
    size_t skip = dfs(d, current_value, target_value, button_pressed, last_index + 1);
    return min(hit, skip);
}

uint64_t check_all_combinations(Diagram* d, size_t current_value, size_t target_value) {
    uint64_t min_presses = UINT8_MAX;
    uint64_t total_combinations = 1 << (d->button_semantics.count - 1);

    for (uint64_t i = 1; i < (1 << d->button_semantics.count); ++i) {
        unsigned presses = (unsigned)__builtin_popcountll(i);

        if (presses >= min_presses)
            continue;

        uint64_t c = i;
        uint64_t value = 0;

        while (c) {
            uint8_t index = __builtin_ctzll(c);
            value ^= d->button_semantics.items[index];

            c &= c - 1;
        }

        if (value == target_value) {
            min_presses = min(min_presses, presses);
        }
    }
    return min_presses;
}

uint64_t shortest_combination(Diagram* d) {
    size_t target_value = d->light_diagram;
    size_t diagram_len = d->light_diagram_len;
    size_t current_value = 0;
    // return dfs(d, current_value, target_value, 0, 0);
    return check_all_combinations(d, current_value, target_value);
}

uint64_t solve(const InputData* lines) {
    Diagrams diagrams = {0};
    for (size_t line_idx = 0; line_idx < lines->count; ++line_idx) {
        char* line = lines->items[line_idx];
        printf("%s\n", line);

        Strings out = {0};
        split(&out, line, " ");

        size_t diagram_len = strlen(out.items[0]) - 2;
        size_t light_diagram = diagram_to_value(out.items[0]);

        ButtonSemantics button_semantics = {0};

        for (size_t i = 1; i < out.count - 1; ++i) {
            size_t button_semantic = semantic_to_value(out.items[i], diagram_len - 1);
            da_append(&button_semantics, button_semantic);
        }

        JoltageReqs reqs = {0};
        char* reqs_str = out.items[out.count - 1];
        char* new_req_str = extract_segment(reqs_str, 1, strlen(reqs_str) - 1);
        Strings req_vals = {0};
        split(&req_vals, new_req_str, ",");

        for (size_t i = 0; i < req_vals.count; ++i) {
            size_t value = (size_t)strtoll(req_vals.items[i], NULL, 10);
            da_append(&reqs, value);
        }

        free(new_req_str);
        da_free(out);
        da_free(req_vals);

        Diagram d = (Diagram){light_diagram, diagram_len, button_semantics, reqs};
        da_append(&diagrams, d);
    }

    uint64_t total = 0;
    uint64_t total_time_taken = 0;
    for (int i = 0; i < 2; ++i) {
        uint64_t start = ns_now();
        total = 0;
        for (size_t idx = 0; idx < diagrams.count; ++idx) {
            uint64_t current_shortest = shortest_combination(&diagrams.items[idx]);
            total += current_shortest;
        }
        uint64_t end = ns_now();
        total_time_taken += (end - start);
    }

    da_free(diagrams);
    for (size_t idx = 0; idx < diagrams.count; ++idx) {
        da_free(diagrams.items[idx].button_semantics);
        da_free(diagrams.items[idx].joltage_requirements);
    }

    printf("Average Elapsed Time (ns) : %lu\n", total_time_taken / 2);
    return total;
}

uint64_t solve_part_2(const InputData* diagrams) {
}

int main() {
    char* input_file = "inputs/q10_input.txt";

    InputData input_lines = read_lines(input_file);

    uint64_t password = solve(&input_lines);
    // uint64_t password = solve_part_2(&input_lines);
    printf("Password : %" PRIu64 "\n", password);
    return 0;
}