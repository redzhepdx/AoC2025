#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct {
    Strings* items;
    size_t count;
    size_t capacity;
} SMatrix;

static char*
strdup_s(const char* s) {
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

typedef struct {
    size_t row;
    size_t col;
} Coord2D;

typedef struct {
    Coord2D* items;
    size_t count;
    size_t capacity;
} Coord2DArray;

uint64_t solve(const InputData* grid) {
    size_t row_count = grid->count;
    size_t col_count = strlen(grid->items[0]);

    Coord2D start_coord = {0};
    Coord2DArray beams = {0};

    struct {
        Coord2D key;
        int value;
    }* beam_hashes = NULL;

    // Find S in the first line
    for (size_t col = 0; col < col_count; ++col) {
        if (grid->items[0][col] == 'S') {
            start_coord = (Coord2D){0, col};
            da_append(&beams, start_coord);
            hmput(beam_hashes, start_coord, 1);
            break;
        }
    }

    uint64_t split_count = 0;

    while (true) {
        Coord2DArray next_beams = {0};
        for (size_t beam_idx = 0; beam_idx < beams.count; ++beam_idx) {
            Coord2D current = beams.items[beam_idx];

            if (current.row < row_count - 1 && current.col < col_count && current.col >= 0) {
                if (grid->items[current.row + 1][current.col] == '.') {
                    Coord2D next = (Coord2D){current.row + 1, current.col};
                    if (hmgeti(beam_hashes, next) == -1) {
                        da_append(&next_beams, next);
                        hmput(beam_hashes, next, 1);
                    }
                } else if (grid->items[current.row + 1][current.col] == '^') {
                    Coord2D left_next = (Coord2D){current.row + 1, current.col - 1};
                    Coord2D right_next = (Coord2D){current.row + 1, current.col + 1};

                    split_count++;

                    if (hmgeti(beam_hashes, left_next) == -1) {
                        da_append(&next_beams, left_next);
                        hmput(beam_hashes, left_next, 1);
                    }

                    if (hmgeti(beam_hashes, right_next) == -1) {
                        da_append(&next_beams, right_next);
                        hmput(beam_hashes, right_next, 1);
                    }
                }
            }
        }
        if (next_beams.count == 0) {
            break;
        }
        da_free(beams);
        beams = next_beams;
    }

    da_free(beams);
    // shfree(beam_hashes);
    hmfree(beam_hashes);

    return split_count;
}

uint64_t solve_part_2(InputData* grid) {
    size_t row_count = grid->count;
    size_t col_count = strlen(grid->items[0]);

    Coord2D start_coord = {0};
    Coord2DArray beams = {0};

    struct {
        Coord2D key;
        uint64_t value;
    }* beam_hashes = NULL;

    // Find S in the first line
    for (size_t col = 0; col < col_count; ++col) {
        if (grid->items[0][col] == 'S') {
            start_coord = (Coord2D){0, col};
            da_append(&beams, start_coord);
            hmput(beam_hashes, start_coord, 1);
            break;
        }
    }

    uint64_t ways_count = 0;

    while (true) {
        Coord2DArray next_beams = {0};
        for (size_t beam_idx = 0; beam_idx < beams.count; ++beam_idx) {
            Coord2D current = beams.items[beam_idx];
            uint64_t path_to_here = hmget(beam_hashes, current);

            if (current.row < row_count - 1 && current.col < col_count && current.col >= 0) {
                if (grid->items[current.row + 1][current.col] == '.') {
                    Coord2D next = (Coord2D){current.row + 1, current.col};
                    if (hmgeti(beam_hashes, next) == -1) {
                        da_append(&next_beams, next);
                        hmput(beam_hashes, next, path_to_here);
                    } else {
                        hmput(beam_hashes, next, hmget(beam_hashes, next) + path_to_here);
                    }
                } else if (grid->items[current.row + 1][current.col] == '^') {
                    Coord2D left_next = (Coord2D){current.row + 1, current.col - 1};
                    Coord2D right_next = (Coord2D){current.row + 1, current.col + 1};

                    if (hmgeti(beam_hashes, left_next) == -1) {
                        da_append(&next_beams, left_next);
                        hmput(beam_hashes, left_next, path_to_here);
                    } else {
                        hmput(beam_hashes, left_next, hmget(beam_hashes, left_next) + path_to_here);
                    }

                    if (hmgeti(beam_hashes, right_next) == -1) {
                        da_append(&next_beams, right_next);
                        hmput(beam_hashes, right_next, path_to_here);
                    } else {
                        hmput(beam_hashes, right_next, hmget(beam_hashes, right_next) + path_to_here);
                    }
                }
            } else if (current.row == row_count - 1) {
                ways_count += hmget(beam_hashes, current);
            }
        }
        if (next_beams.count == 0) {
            break;
        }
        beams = next_beams;
    }

    return ways_count;
}

SMatrix read_matrix(const InputData* input_lines) {
    size_t row_count = input_lines->count;
    SMatrix matrix = {0};
    for (size_t row = 0; row < row_count; ++row) {
        Strings out = {0};
        split(&out, input_lines->items[row], " ");
        da_append(&matrix, out);
    }
    return matrix;
}

int main() {
    char* input_file = "inputs/q7_input.txt";

    InputData input_lines = read_lines(input_file);

    uint64_t password = solve(&input_lines);
    uint64_t password = solve_part_2(&input_lines);
    printf("Password : %" PRIu64 "\n", password);

    da_free(input_lines);
    return 0;
}