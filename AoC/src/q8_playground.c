#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX

#include "../header/disjoint_set.h"
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
    int64_t x, y, z;
} Point;

typedef struct {
    size_t p1_index;
    size_t p2_index;
    int64_t distance;
} Edge;

typedef struct
{
    Edge* items;
    size_t count;
    size_t capacity;
} EdgeArray;

typedef struct {
    Point* items;
    size_t count;
    size_t capacity;
} PointArray;

int compare_edges(const void* x, const void* y) {
    Edge e1 = *(Edge*)x;
    Edge e2 = *(Edge*)y;
    if (e1.distance < e2.distance) {
        return -1;
    }
    if (e1.distance > e2.distance) {
        return 1;
    }
    return 0;
}

int cmp_size_t(const void* a, const void* b) {
    size_t x = *(const size_t*)a;
    size_t y = *(const size_t*)b;

    if (x > y) return -1;
    if (x < y) return 1;
    return 0;
}

uint64_t solve(const InputData* grid, size_t max_iterations) {
    size_t row_count = grid->count;
    size_t col_count = strlen(grid->items[0]);

    PointArray points = {0};

    for (size_t point_index = 0; point_index < row_count; ++point_index) {
        char* point_str = grid->items[point_index];
        Strings coordinates = {0};
        split(&coordinates, point_str, ",");

        int32_t x = (int32_t)strtoull(coordinates.items[0], NULL, 10);
        int32_t y = (int32_t)strtoull(coordinates.items[1], NULL, 10);
        int32_t z = (int32_t)strtoull(coordinates.items[2], NULL, 10);

        Point p = (Point){x, y, z};
        da_append(&points, p);
        da_free(coordinates);
    }

    EdgeArray edges = {0};
    for (size_t p_index = 0; p_index < points.count - 1; ++p_index) {
        for (size_t other_p_index = p_index + 1; other_p_index < points.count; ++other_p_index) {
            Point p1 = points.items[p_index];
            Point p2 = points.items[other_p_index];

            int64_t diff_sq_x = (p1.x - p2.x) * (p1.x - p2.x);
            int64_t diff_sq_y = (p1.y - p2.y) * (p1.y - p2.y);
            int64_t diff_sq_z = (p1.z - p2.z) * (p1.z - p2.z);
            int64_t distance = diff_sq_x + diff_sq_y + diff_sq_z;

            Edge edge = (Edge){p_index, other_p_index, distance};
            da_append(&edges, edge);
        }
    }

    qsort(edges.items, edges.count, sizeof(Edge), compare_edges);

    DSU set = {0};
    dsu_init(&set, points.count);

    for (size_t edge_idx = 0; edge_idx < max_iterations; ++edge_idx) {
        size_t p1_index = edges.items[edge_idx].p1_index;
        size_t p2_index = edges.items[edge_idx].p2_index;
        float distance = edges.items[edge_idx].distance;

        if (dsu_find(&set, p1_index) != dsu_find(&set, p2_index)) {
            dsu_union(&set, p1_index, p2_index);
        }
    }

    size_t* sizes = (size_t*)calloc(points.count, sizeof(size_t));
    for (size_t pidx = 0; pidx < points.count; ++pidx) {
        sizes[dsu_find(&set, pidx)] += 1;
    }

    qsort(set.size, points.count, sizeof(size_t), cmp_size_t);
    printf("Top 3 Set Sizes : %d %d %d\n", set.size[0], set.size[1], set.size[2]);

    da_free(points);
    da_free(edges);
    dsu_free(&set);

    return (uint64_t)set.size[0] * (uint64_t)set.size[1] * (uint64_t)set.size[2];
}

uint64_t solve_part_2(const InputData* grid) {
    size_t row_count = grid->count;
    size_t col_count = strlen(grid->items[0]);

    PointArray points = {0};

    for (size_t point_index = 0; point_index < row_count; ++point_index) {
        char* point_str = grid->items[point_index];
        Strings coordinates = {0};
        split(&coordinates, point_str, ",");

        int32_t x = (int32_t)strtoull(coordinates.items[0], NULL, 10);
        int32_t y = (int32_t)strtoull(coordinates.items[1], NULL, 10);
        int32_t z = (int32_t)strtoull(coordinates.items[2], NULL, 10);

        Point p = (Point){x, y, z};
        da_append(&points, p);

        da_free(coordinates);
    }

    EdgeArray edges = {0};
    for (size_t p_index = 0; p_index < points.count - 1; ++p_index) {
        for (size_t other_p_index = p_index + 1; other_p_index < points.count; ++other_p_index) {
            Point p1 = points.items[p_index];
            Point p2 = points.items[other_p_index];

            int64_t diff_sq_x = (p1.x - p2.x) * (p1.x - p2.x);
            int64_t diff_sq_y = (p1.y - p2.y) * (p1.y - p2.y);
            int64_t diff_sq_z = (p1.z - p2.z) * (p1.z - p2.z);
            int64_t distance = diff_sq_x + diff_sq_y + diff_sq_z;
            Edge edge = (Edge){p_index, other_p_index, distance};
            da_append(&edges, edge);
        }
    }

    qsort(edges.items, edges.count, sizeof(Edge), compare_edges);

    DSU set = {0};
    dsu_init(&set, points.count);

    size_t connection_count = 0;

    for (size_t edge_idx = 0; edge_idx < edges.count; ++edge_idx) {
        size_t p1_index = edges.items[edge_idx].p1_index;
        size_t p2_index = edges.items[edge_idx].p2_index;
        float distance = edges.items[edge_idx].distance;

        if (dsu_find(&set, p1_index) != dsu_find(&set, p2_index)) {
            dsu_union(&set, p1_index, p2_index);
            connection_count++;
        }

        if (connection_count == points.count - 1) {
            printf("%" PRIi64 " %" PRIi64 " \n", points.items[p1_index].x, points.items[p2_index].x);
            return points.items[p1_index].x * points.items[p2_index].x;
        }
    }

    da_free(points);
    da_free(edges);
    dsu_free(&set);

    return -1;
}

int main() {
    char* input_file = "inputs/q8_input.txt";

    InputData input_lines = read_lines(input_file);

    uint64_t password = solve(&input_lines, 1000);
    uint64_t password = solve_part_2(&input_lines);
    printf("Password : %" PRIu64 "\n", password);

    da_free(input_lines);
    return 0;
}