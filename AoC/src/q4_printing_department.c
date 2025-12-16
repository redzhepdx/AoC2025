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

bool is_in_grid(const int row, const int col, const int grid_row, const int grid_col) {
    return row >= 0 && col >= 0 && row < grid_row && col < grid_col;
}

uint64_t solve(const InputData* grid, InputData* grid_next, bool vis) {
    int column_size = strlen(grid->items[0]);
    int row_size = grid->count;
    uint64_t total_collectable_roll = 0;

    if (vis) {
        for (int row = 0; row < row_size; ++row) {
            for (int col = 0; col < column_size; col++) {
                printf("%c", grid->items[row][col]);
            }
            printf("\n");
        }
    }

    for (int row = 0; row < row_size; ++row) {
        for (int col = 0; col < column_size; col++) {
            uint64_t adjacent_roll_count = 0;

            if (grid->items[row][col] != '@')
                continue;

            for (int ng_r = -1; ng_r <= 1; ng_r++) {
                for (int ng_c = -1; ng_c <= 1; ng_c++) {
                    int ng_cell_r = row + ng_r;
                    int ng_cell_c = col + ng_c;

                    // If it is a different cell
                    if ((ng_cell_r != row || ng_cell_c != col)) {
                        // If it is in the grid
                        if (is_in_grid(ng_cell_r, ng_cell_c, row_size, column_size)) {
                            // If it is a roll
                            if (grid->items[ng_cell_r][ng_cell_c] == '@') {
                                adjacent_roll_count++;
                            }
                        }
                    }
                }
            }

            if (adjacent_roll_count < 4) {
                grid_next->items[row][col] = '.';
                total_collectable_roll++;
            }
        }
    }

    return total_collectable_roll;
}

uint64_t solve_part_2(InputData* grid) {
    InputData* grid_next = copy_input_data(grid);
    uint64_t cleaned_num_rolls = solve(grid, grid_next, true);
    uint64_t cleaned_total_rolls = cleaned_num_rolls;
    while (cleaned_num_rolls != 0) {
        grid = grid_next;
        cleaned_num_rolls = solve(grid, grid_next, false);
        cleaned_total_rolls += cleaned_num_rolls;
    }
    return cleaned_total_rolls;
}

int main() {
    char* input_file = "inputs/q4_input.txt";

    InputData input_lines = read_lines(input_file);
    InputData* input_lines_cpy = copy_input_data(&input_lines);

    uint64_t password = solve(&input_lines, input_lines_cpy, false);
    uint64_t password = solve_part_2(&input_lines);
    printf("Password : %" PRIu64 "\n", password);

    da_free(input_lines);
    return 0;
}