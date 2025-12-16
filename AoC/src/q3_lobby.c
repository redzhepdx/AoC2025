#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define LINE_MAX_LEN 2024
typedef struct {
    char** items;
    size_t count;
    size_t capacity;
} Strings;

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

uint64_t solve(const InputData joltage_rating_lines) {
    uint64_t total_max_joltages = 0;
    for (uint64_t line_idx = 0; line_idx < joltage_rating_lines.count; ++line_idx) {
        char* joltage_ratings = joltage_rating_lines.items[line_idx];
        size_t len = strlen(joltage_ratings);

        uint64_t max_joltage = 0;
        uint64_t max_first = (uint64_t)char_to_int(joltage_ratings[0]);
        printf("Line :%s\n", joltage_ratings);

        for (uint64_t i = 1; i < len; ++i) {
            uint64_t current_jolt_rating = (uint64_t)char_to_int(joltage_ratings[i]);
            max_joltage = max(max_first * 10 + current_jolt_rating, max_joltage);
            max_first = max(current_jolt_rating, max_first);
        }

        printf("%s - %" PRIu64 " - %" PRIu64 "\n", joltage_ratings, max_joltage, max_first);

        total_max_joltages += max_joltage;
    }
    return total_max_joltages;
}

uint64_t solve_part_2(const InputData joltage_rating_lines, size_t num_digits) {
    uint64_t total_max_joltages = 0;
    for (uint64_t line_idx = 0; line_idx < joltage_rating_lines.count; ++line_idx) {
        char* joltage_ratings = joltage_rating_lines.items[line_idx];

        size_t len = strlen(joltage_ratings);
        size_t current_segment_len = num_digits;

        size_t start = 0;
        uint64_t max_joltage = 0;

        while (current_segment_len > 0) {
            // Find the first max value in this segment
            uint64_t current_max = 0;
            size_t segment_end = len - current_segment_len + 1;

            for (size_t segment_idx = start; segment_idx < segment_end; ++segment_idx) {
                uint64_t current_value = (uint64_t)char_to_int(joltage_ratings[segment_idx]);
                if (current_max < current_value) {
                    current_max = current_value;
                    start = segment_idx + 1;
                }
                current_max = max(current_max, current_value);
            }
            max_joltage += current_max * (uint64_t)pow(10, current_segment_len - 1);

            current_segment_len--;
        }

        total_max_joltages += max_joltage;
    }
    return total_max_joltages;
}

int main() {
    char* input_file = "inputs/q3_input.txt";

    InputData input_lines = read_lines(input_file);

    uint64_t password = solve(input_lines);
    uint64_t password = solve_part_2(input_lines, 12);
 
    printf("Password : %" PRIu64 "\n", password);

    da_free(input_lines);

    return 0;
}