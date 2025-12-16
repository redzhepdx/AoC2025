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

void split_v2(Strings* out, const char* text, const char* delimiter) {
    if (!text || !delimiter || !delimiter[0]) {
        return;
    }

    char delim = delimiter[0];  // assume single char delimiter, e.g. ' '

    const char* p = text;
    const char* token_start = p;
    int seen_non_delim = 0;  // have we seen a non-space in this token?

    while (*p) {
        if (*p == delim) {
            if (seen_non_delim) {
                // We just hit the *first* delimiter after some non-delim chars:
                // cut the token here, remove exactly ONE delimiter.
                size_t len = (size_t)(p - token_start);
                if (len > 0) {
                    char* tok_copy = malloc(len + 1);
                    if (!tok_copy) {
                        fprintf(stderr, "Failed to allocate memory for token\n");
                        return;
                    }
                    memcpy(tok_copy, token_start, len);
                    tok_copy[len] = '\0';
                    da_append(out, tok_copy);
                }

                // Consume exactly one delimiter, leave the rest (if any)
                p++;
                token_start = p;
                seen_non_delim = 0;
                continue;  // continue scanning from here, but now in a new token
            } else {
                // Still haven't seen a non-delim in this token yet
                // (e.g. leading spaces before first real char of this token).
                // Keep these as part of the token.
                p++;
            }
        } else {
            seen_non_delim = 1;
            p++;
        }
    }

    // Last token (only if it has non-delim characters)
    if (seen_non_delim) {
        size_t len = (size_t)(p - token_start);
        if (len > 0) {
            char* tok_copy = malloc(len + 1);
            if (!tok_copy) {
                fprintf(stderr, "Failed to allocate memory for token\n");
                return;
            }
            memcpy(tok_copy, token_start, len);
            tok_copy[len] = '\0';
            da_append(out, tok_copy);
        }
    }
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

uint64_t solve(const SMatrix* grid) {
    size_t row_count = grid->count;
    size_t col_count = grid->items[0].count;

    uint64_t total = 0;

    for (size_t col = 0; col < col_count; ++col) {
        char* op = grid->items[row_count - 1].items[col];
        char op_ch = strcmp(op, "+") == 0 ? '+' : '*';
        uint64_t col_value = 1;
        
        if (op_ch == '+') {
            col_value = 0;
        }
        
        for (size_t row = 0; row < row_count - 1; ++row) {
            uint64_t value = (uint64_t)strtoull(grid->items[row].items[col], NULL, 10);
            printf("%s\n", grid->items[row].items[col]);

            if (op_ch == '+') {
                col_value += value;
            } else if (op_ch == '*') {
                col_value *= value;
            }
        }
        total += col_value;
        printf("\nTotal = %" PRIu64 "\nGrandTotal = %" PRIu64 "\n", col_value, total);
    }
    return total;
}

uint64_t solve_part_2(SMatrix* grid) {
    // This could have been cleaner but it is what it is. Nothing to gain as an algorithmic knowledge unless you want to verify your solution.
    size_t row_count = grid->count;
    size_t col_count = grid->items[0].count;

    uint64_t total = 0;

    for (size_t col = 0; col < col_count; ++col) {
        char* op = grid->items[row_count - 1].items[col];
        remove_spaces(op);
        char op_ch = strcmp(op, "+") == 0 ? '+' : '*';
        uint64_t col_value = 1;

        if (op_ch == '+') {
            col_value = 0;
        }

        // Find the longest string to be able to loop vertically in the second for loop of alignment section
        size_t max_word_len = 0;
        bool check_extra_space = true;

        for (size_t row = 0; row < row_count - 1; ++row) {
            char* word = grid->items[row].items[col];

            size_t len_word = strlen(word);
            max_word_len = max(max_word_len, len_word);

            if (word[0] != ' ') {
                check_extra_space = false;
            }
        }

        if (check_extra_space) {
            for (size_t row = 0; row < row_count - 1; ++row) {
                char* word = grid->items[row].items[col];
                char* word_cpy = temp_strdup(word);
                remove_spaces(word_cpy);
                if (strlen(word_cpy) == max_word_len - 1) {
                    word += 1;
                    grid->items[row].items[col] += 1;
                }
            }
        }

        // ALIGNMENT : Read the numbers vertically
        Strings top_down_numbers = {0};
        for (size_t number_index = 0; number_index < max_word_len; ++number_index) {
            char* num = (char*)malloc(row_count - 1 * sizeof(char));
            for (size_t row = 0; row < row_count - 1; ++row) {
                char* word = grid->items[row].items[col];
                size_t len_word = strlen(word);
                char word_ch = ' ';
                if (number_index < len_word) {
                    word_ch = word[number_index];
                }
                num[row] = word_ch;
            }
            remove_spaces(num);
            if (num[0] != '\0') {
                da_append(&top_down_numbers, nob_temp_strdup(num));
            }
            free(num);
        }

        // Calculate the math problem
        for (size_t num_idx = 0; num_idx < top_down_numbers.count; ++num_idx) {
            uint64_t value = (uint64_t)strtoull(top_down_numbers.items[num_idx], NULL, 10);
            if (op_ch == '+') {
                col_value += value;
            } else if (op_ch == '*') {
                col_value *= value;
            }
        }

        total += col_value;
    }
    return total;
}

SMatrix read_matrix(const InputData* input_lines) {
    size_t row_count = input_lines->count;
    SMatrix matrix = {0};
    for (size_t row = 0; row < row_count; ++row) {
        Strings out = {0};
        split_v2(&out, input_lines->items[row], " ");
        da_append(&matrix, out);
    }

    return matrix;
}

int main() {
    char* input_file = "inputs/q6_input.txt";

    InputData input_lines = read_lines(input_file);
    SMatrix grid = read_matrix(&input_lines);

    uint64_t password = solve(&grid);
    uint64_t password = solve_part_2(&grid);
    printf("Password : %" PRIu64 "\n", password);

    size_t row_count = input_lines.count;
    for (size_t row = 0; row < row_count; ++row) {
        da_free(grid.items[row]);
    }

    da_free(input_lines);
    da_free(grid);

    return 0;
}