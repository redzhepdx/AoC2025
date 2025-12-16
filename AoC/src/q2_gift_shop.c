#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX

#include "../header/nob.h"

typedef long long ull;

typedef struct {
    int start;
    int end;
} IDPair;

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

// This is for part I
int is_valid(char* value_str) {
    size_t len = strlen(value_str);

    if (len % 2 != 0) {
        return 1;
    }

    size_t half = len / 2;

    char* left = malloc(half + 1);
    char* right = malloc(half + 1);

    memcpy(left, value_str, half);
    memcpy(right, value_str + half, half);

    left[half] = '\0';
    right[half] = '\0';

    if (strcmp(left, right) == 0) {
        return 0;
    }

    return 1;
}

void split_into_chunks(Strings* out, const char* text, size_t chunk_size) {
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

// This is for part II
int is_valid_v2(char* value_str) {
    size_t len = strlen(value_str);

    for (int chunk_size = 1; chunk_size <= len / 2; ++chunk_size) {
        if (len % chunk_size == 0) {
            Strings segments = {0};
            split_into_chunks(&segments, value_str, chunk_size);

            bool invalid = true;
            for (int seg_idx = 1; seg_idx < segments.count; ++seg_idx) {
                if (strcmp(segments.items[seg_idx - 1], segments.items[seg_idx]) != 0) {
                    invalid = false;
                    break;
                }
            }

            if (invalid) {
                return 0;
            }
            
            da_free(segments);
        }
    }

    return 1;
}

uint64_t solve(Strings id_pairs) {
    uint64_t sum_of_invalid_ids = 0;

    for (size_t i = 0; i < id_pairs.count; ++i) {
        Strings id_pair = {0};
        split(&id_pair, id_pairs.items[i], "-");
        
        if (id_pair.items[0][0] == "0" || id_pair.items[1][0] == "0") {
            continue;
        }
        
        uint64_t low = (uint64_t)strtoull(id_pair.items[0], NULL, 10);
        uint64_t high = (uint64_t)strtoull(id_pair.items[1], NULL, 10);
        
        for (uint64_t value = low; value <= high; ++value) {
            char value_str[32];
            sprintf(value_str, "%zu", value);
            if (!is_valid_v2(value_str)) {
                sum_of_invalid_ids += value;
            }
        }

        da_free(id_pair);
    }

    return sum_of_invalid_ids;
}

int main() {
    char* input_file = "inputs/q2_input.txt";

    InputData input_lines = read_lines(input_file);

    Strings strings = {0};
    split(&strings, input_lines.items[0], ",");

    uint64_t password = solve(strings);
    printf("Password : %" PRIu64 "\n", password);

    da_free(input_lines);
    da_free(strings);

    return 0;
}