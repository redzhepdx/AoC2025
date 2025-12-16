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
    int64_t x, y;
} Point;

typedef struct {
    Point p1;
    Point p2;
} Edge;

typedef struct {
    Edge* items;
    size_t count;
    size_t capacity;
} EdgeArray;

typedef struct {
    Point* items;
    size_t count;
    size_t capacity;
} PointArray;

int cmp_size_t(const void* a, const void* b) {
    size_t x = *(const size_t*)a;
    size_t y = *(const size_t*)b;

    if (x > y) return -1;
    if (x < y) return 1;
    return 0;
}

uint64_t solve(const InputData* coords) {
    size_t row_count = coords->count;
    size_t col_count = strlen(coords->items[0]);

    PointArray points = {0};

    for (size_t point_index = 0; point_index < row_count; ++point_index) {
        char* point_str = coords->items[point_index];
        Strings coordinates = {0};
        split(&coordinates, point_str, ",");

        int32_t x = (int32_t)strtoull(coordinates.items[0], NULL, 10);
        int32_t y = (int32_t)strtoull(coordinates.items[1], NULL, 10);

        Point p = (Point){x, y};
        da_append(&points, p);
        da_free(coordinates);
    }
    uint64_t max_rect_area = 0;
    for (size_t p_idx = 0; p_idx < row_count - 1; ++p_idx) {
        for (size_t p_o_idx = p_idx + 1; p_o_idx < row_count; ++p_o_idx) {
            Point p1 = points.items[p_idx];
            Point p2 = points.items[p_o_idx];
            uint64_t len_row = abs(p1.x - p2.x + 1);
            uint64_t len_col = abs(p1.y - p2.y + 1);
            max_rect_area = max(max_rect_area, len_col * len_row);
        }
    }

    da_free(points);
    return max_rect_area;
}

static inline int orient_sign(Point a, Point b, Point c) {
    __int128 x1 = (__int128)b.x - a.x;
    __int128 y1 = (__int128)b.y - a.y;
    __int128 x2 = (__int128)c.x - a.x;
    __int128 y2 = (__int128)c.y - a.y;

    __int128 cross = x1 * y2 - y1 * x2;
    if (cross > 0) return 1;
    if (cross < 0) return -1;
    return 0;
}

static inline int point_on_segment(Point p, Point a, Point b) {
    if (orient_sign(a, b, p) != 0) return 0;
    if (p.x < (a.x < b.x ? a.x : b.x)) return 0;
    if (p.x > (a.x > b.x ? a.x : b.x)) return 0;
    if (p.y < (a.y < b.y ? a.y : b.y)) return 0;
    if (p.y > (a.y > b.y ? a.y : b.y)) return 0;
    return 1;
}

int segments_intersect_strict(Point p1, Point p2, Point p3, Point p4) {
    int o1 = orient_sign(p1, p2, p3);
    int o2 = orient_sign(p1, p2, p4);
    int o3 = orient_sign(p3, p4, p1);
    int o4 = orient_sign(p3, p4, p2);

    // Proper intersection: both pairs strictly straddle
    if ((o1 * o2 < 0) && (o3 * o4 < 0))
        return 1;

    // collinear / touching / disjoint => not a "strict" crossing
    return 0;
}

int point_in_polygon_inclusive(Point p, const PointArray* poly) {
    for (int i = 0, j = poly->count - 1; i < poly->count; j = i++) {
        if (point_on_segment(p, poly->items[j], poly->items[i]))
            // on boundary => inside
            return 1;
    }

    // Ray casting to +infinity in x direction
    int inside = 0;
    double px = (double)p.x;
    double py = (double)p.y;

    for (int i = 0, j = poly->count - 1; i < poly->count; j = i++) {
        double xi = (double)poly->items[i].x;
        double yi = (double)poly->items[i].y;
        double xj = (double)poly->items[j].x;
        double yj = (double)poly->items[j].y;

        int intersect = ((yi > py) != (yj > py));
        if (intersect) {
            double x_intersect = xi + (py - yi) * (xj - xi) / (yj - yi);
            if (x_intersect > px)
                inside = !inside;
        }
    }

    return inside;
}


int rectangle_inside_polygon(Point p1, Point p2, const PointArray* poly) {
    int64_t minx = (p1.x < p2.x) ? p1.x : p2.x;
    int64_t maxx = (p1.x > p2.x) ? p1.x : p2.x;
    int64_t miny = (p1.y < p2.y) ? p1.y : p2.y;
    int64_t maxy = (p1.y > p2.y) ? p1.y : p2.y;

    Point rect[4] = {
        {minx, miny},
        {minx, maxy},
        {maxx, maxy},
        {maxx, miny}};

    for (int i = 0; i < 4; ++i) {
        if (!point_in_polygon_inclusive(rect[i], poly))
            return 0;
    }

    for (int ri = 0; ri < 4; ++ri) {
        Point r1 = rect[ri];
        Point r2 = rect[(ri + 1) & 3];

        for (int i = 0, j = poly->count - 1; i < poly->count; j = i++) {
            Point pA = poly->items[j];
            Point pB = poly->items[i];

            if (segments_intersect_strict(r1, r2, pA, pB)) {
                // Edge is outside of the polygon
                return 0;
            }
        }
    }

    return 1;
}

uint64_t solve_part_2(const InputData* coords) {
    size_t row_count = coords->count;
    size_t col_count = strlen(coords->items[0]);

    PointArray points = {0};
    EdgeArray edges = {0};

    for (size_t point_index = 0; point_index < row_count; ++point_index) {
        char* point_str = coords->items[point_index];
        Strings coordinates = {0};
        split(&coordinates, point_str, ",");

        int64_t x = (int64_t)strtoull(coordinates.items[0], NULL, 10);
        int64_t y = (int64_t)strtoull(coordinates.items[1], NULL, 10);

        Point p = (Point){x, y};

        da_append(&points, p);
        da_free(coordinates);
    }

    uint64_t max_area_size = 0;

    for (size_t pidx = 0; pidx < points.count - 1; ++pidx) {
        for (size_t pidy = pidx + 2; pidy < points.count; ++pidy) {
            Point p1 = points.items[pidx];
            Point p2 = points.items[pidy];

            int64_t len_1 = abs(p1.x - p2.x) + 1;
            int64_t len_2 = abs(p1.y - p2.y) + 1;

            uint64_t area = len_1 * len_2;
            if (area <= max_area_size)
                continue;

            int rect_is_in = rectangle_inside_polygon(p1, p2, &points);

            if (rect_is_in) {
                max_area_size = area;
                printf("New Points x1: %" PRIi64 " y1: %" PRIi64 " x2 : %" PRIi64 " y2 : %" PRIi64 " Len1 : %" PRIi64 " Len2: %" PRIi64 "\n", p1.x, p1.y, p2.x, p2.y, len_1, len_2);
            }
        }
    }
    
    da_free(points);
    da_free(edges);
    
    return max_area_size;
}

int main() {
    char* input_file = "inputs/q9_input.txt";

    InputData input_lines = read_lines(input_file);

    uint64_t password = solve(&input_lines);
    uint64_t password = solve_part_2(&input_lines);
    printf("Password : %" PRIu64 "\n", password);

    da_free(input_lines);
    return 0;
}