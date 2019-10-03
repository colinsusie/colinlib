#include "co_queue.h"

#define INIT_SIZE 8

void coqueue_init(coqueue_t *queue, uint16_t itemsize) {
    queue->head = 0;
    queue->tail = 0;
    covec_init(&queue->vec, itemsize);
    covec_resize(&queue->vec, INIT_SIZE);
}

void coqueue_free(coqueue_t *queue) {
    covec_free(&queue->vec);
}

int coqueue_size(coqueue_t *queue) {
    int head = queue->head;
    int tail = queue->tail;
    return tail >= head ? tail - head : covec_size(&queue->vec) - (head - tail); 
}

static void _check_and_grow(coqueue_t *queue) {
    int size = coqueue_size(queue);
    int vecsize = queue->vec.size;
    if (size + 1 >= vecsize) {
        covec_resize(&queue->vec, vecsize * 2);
        if (queue->tail < queue->head) {
            int count = vecsize - queue->head;
            int newhead = queue->vec.size - count;
            covec_move(&queue->vec, queue->head, newhead, count);
            queue->head = newhead;
        }
    }
}

void coqueue_push(coqueue_t *queue, const void *data) {
    _check_and_grow(queue);
    covec_set_at(&queue->vec, queue->tail, data);
    queue->tail = (queue->tail + 1) % queue->vec.size;
}

bool coqueue_pop(coqueue_t *queue, void *data) {
    int size = coqueue_size(queue);
    if (size) {
        covec_get_at(&queue->vec, queue->head, data);
        queue->head = (queue->head + 1) % queue->vec.size;
        return true;
    }
    return false;
}

bool coqueue_peek(coqueue_t *queue, int index, void *data) {
    int size = coqueue_size(queue);
    if (index < 0)
        index = size + index;
    if (index < 0 || index >= size)
        return false;
    index = (queue->head + index) % queue->vec.size;
    covec_get_at(&queue->vec, index, data);
    return true;
}

