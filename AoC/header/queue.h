#ifndef GENERIC_QUEUE_H
#define GENERIC_QUEUE_H

#include <stddef.h>  // for size_t
#include <stdlib.h>
#include <string.h>

typedef struct Queue {
    void* data;        // raw buffer
    size_t elem_size;  // size of each element
    size_t capacity;   // total number of elements buffer can hold
    size_t head;       // index of first element
    size_t tail;       // index one past the last element
    size_t size;       // current number of elements
} Queue;

int queue_init(Queue* q, size_t elem_size, size_t initial_capacity) {
    if (!q || elem_size == 0 || initial_capacity == 0) return -1;

    q->data = malloc(elem_size * initial_capacity);
    if (!q->data) return -1;

    q->elem_size = elem_size;
    q->capacity = initial_capacity;
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    return 0;
}

void queue_free(Queue* q) {
    if (!q) return;
    free(q->data);
    q->data = NULL;
    q->elem_size = 0;
    q->capacity = 0;
    q->head = q->tail = q->size = 0;
}

int queue_is_empty(const Queue* q) {
    return q->size == 0;
}

int queue_is_full(const Queue* q) {
    return q->size == q->capacity;
}

static int queue_grow(Queue* q) {
    size_t new_cap = q->capacity * 2;
    void* new_data = malloc(new_cap * q->elem_size);
    if (!new_data) return -1;

    // Copy existing elements in order to new_data starting at index 0
    for (size_t i = 0; i < q->size; ++i) {
        size_t idx = (q->head + i) % q->capacity;
        void* src = (char*)q->data + idx * q->elem_size;
        void* dst = (char*)new_data + i * q->elem_size;
        memcpy(dst, src, q->elem_size);
    }

    free(q->data);
    q->data = new_data;
    q->capacity = new_cap;
    q->head = 0;
    q->tail = q->size;  // one past the last element
    return 0;
}

int queue_enqueue(Queue* q, const void* elem) {
    if (!q || !elem) return -1;

    if (queue_is_full(q)) {
        if (queue_grow(q) != 0) return -1;
    }

    void* dst = (char*)q->data + q->tail * q->elem_size;
    memcpy(dst, elem, q->elem_size);

    q->tail = (q->tail + 1) % q->capacity;
    q->size++;
    return 0;
}

int queue_dequeue(Queue* q, void* out_elem) {
    if (!q || !out_elem) return -1;
    if (queue_is_empty(q)) return -1;  // underflow

    void* src = (char*)q->data + q->head * q->elem_size;
    memcpy(out_elem, src, q->elem_size);

    q->head = (q->head + 1) % q->capacity;
    q->size--;
    return 0;
}

int queue_peek(const Queue* q, void* out_elem) {
    if (!q || !out_elem) return -1;
    if (queue_is_empty(q)) return -1;

    void* src = (char*)q->data + q->head * q->elem_size;
    memcpy(out_elem, src, q->elem_size);
    return 0;
}

/*

#include <stdio.h>
#include "generic_queue.h"

int main(void) {
    Queue qi;
    if (queue_init(&qi, sizeof(int), 4) != 0) {
        fprintf(stderr, "Failed to init int queue\n");
        return 1;
    }

    // Enqueue ints
    for (int i = 0; i < 10; ++i) {
        queue_enqueue(&qi, &i);
    }

    // Dequeue ints
    while (!queue_is_empty(&qi)) {
        int v;
        queue_dequeue(&qi, &v);
        printf("%d\n", v);
    }

    queue_free(&qi);

    // Example with a struct
    typedef struct {
        float x, y;
    } Point;

    Queue qp;
    queue_init(&qp, sizeof(Point), 2);

    Point p;
    p.x = 1.0f; p.y = 2.0f;
    queue_enqueue(&qp, &p);

    p.x = 3.0f; p.y = 4.0f;
    queue_enqueue(&qp, &p);

    while (!queue_is_empty(&qp)) {
        Point out;
        queue_dequeue(&qp, &out);
        printf("(%.1f, %.1f)\n", out.x, out.y);
    }

    queue_free(&qp);

    return 0;
}

*/

#endif  // GENERIC_QUEUE_H
