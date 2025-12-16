#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX

#include "../header/interval_tree.h"
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

uint64_t solve(const InputData* recipe) {
    bool switch_recipe = false;
    uint64_t available_ingredient_count = 0;
    ITNode* root = NULL;

    for (size_t i = 0; i < recipe->count; ++i) {
        if (!switch_recipe) {
            if (strcmp(recipe->items[i], "") == 0) {
                switch_recipe = true;
                printf("Switch");
                continue;
            }

            Strings interval = {0};
            split(&interval, recipe->items[i], "-");

            uint64_t low = (uint64_t)strtoull(interval.items[0], NULL, 10);
            uint64_t high = (uint64_t)strtoull(interval.items[1], NULL, 10);

            root = insert(root, (Interval){low, high});

            da_free(interval);
        } else {
            uint64_t item = (uint64_t)strtoull(recipe->items[i], NULL, 10);
            bool found = containsPoint(root, item, NULL);
            if (found) {
                available_ingredient_count++;
            }
        }
    }
    return available_ingredient_count;
}

uint64_t inorder_sum(ITNode* root) {
    if (!root) return 0;

    uint64_t sum = 0;
    sum += (root->interval.high - root->interval.low) + 1;
    sum += inorder_sum(root->left);
    sum += inorder_sum(root->right);
    return sum;
}

uint64_t solve_part_2(const InputData* recipe) {
    bool switch_recipe = false;
    ITNode* root = NULL;

    for (size_t i = 0; i < recipe->count; ++i) {
        if (!switch_recipe) {
            if (strcmp(recipe->items[i], "") == 0) {
                switch_recipe = true;
                continue;
            }

            Strings interval = {0};
            split(&interval, recipe->items[i], "-");

            uint64_t low = (uint64_t)strtoull(interval.items[0], NULL, 10);
            uint64_t high = (uint64_t)strtoull(interval.items[1], NULL, 10);

            root = insertAndMerge(root, (Interval){low, high});
        } else {
            break;
        }
    }
    // inorder(root);
    return inorder_sum(root);
}

int main() {
    char* input_file = "inputs/q5_input_simple.txt";

    InputData input_lines = read_lines(input_file);

    uint64_t password = solve(&input_lines);
    uint64_t password = solve_part_2(&input_lines);
    printf("Password : %" PRIu64 "\n", password);

    da_free(input_lines);
    return 0;
}