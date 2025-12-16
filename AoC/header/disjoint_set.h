#include <stdio.h>
#include <stdlib.h>

typedef struct {
    size_t n;
    size_t* parent;
    size_t* size;
} DSU;

void dsu_init(DSU* dsu, size_t n) {
    dsu->n = n;
    dsu->parent = (size_t*)malloc(n * sizeof(size_t));
    dsu->size = (size_t*)malloc(n * sizeof(size_t));

    for (size_t i = 0; i < n; i++) {
        dsu->parent[i] = i;
        dsu->size[i] = 1;
    }
}

size_t dsu_find(DSU* dsu, size_t i) {
    if (dsu->parent[i] == i)
        return i;
    return dsu->parent[i] = dsu_find(dsu, dsu->parent[i]);
}

void dsu_union(DSU* dsu, size_t i, size_t j) {
    size_t a = dsu_find(dsu, i);
    size_t b = dsu_find(dsu, j);
    if (a != b) {
        if (dsu->size[a] < dsu->size[b]) {
            dsu->parent[a] = b;
            dsu->size[b] += dsu->size[a];
        } else {
            dsu->parent[b] = a;
            dsu->size[a] += dsu->size[b];
        }
    }
}

void dsu_mix(DSU* dsu, size_t i, size_t j) {
    size_t a = dsu_find(dsu, i);
    size_t b = dsu_find(dsu, j);
    dsu->parent[a] = b;
}

size_t dsu_set_size(DSU* dsu, size_t i) {
    return dsu->size[dsu_find(dsu, i)];
}

void dsu_free(DSU *dsu) {
    free(dsu->parent);
    free(dsu->size);
    dsu->n = 0;
}

/*
size_t main() {
    DSU dsu;
    dsu_init(&dsu, 5);

    dsu_union(&dsu, 0, 1);
    dsu_union(&dsu, 2, 3);
    dsu_union(&dsu, 0, 2);

    printf("Representative of 1: %d\n", dsu_find(&dsu, 1));
    printf("Representative of 3: %d\n", dsu_find(&dsu, 3));

    printf("Are 1 and 3 in the same set? %s\n",
        dsu_find(&dsu, 1) == dsu_find(&dsu, 3) ? "Yes" : "No");

    printf("Size of the set containing 1: %d\n", dsu_set_size(&dsu, 1));
    printf("Size of the set containing 4: %d\n", dsu_set_size(&dsu, 4));

    dsu_free(&dsu);
    return 0;
}
*/