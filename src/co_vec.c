#include "co_vec.h"

#define INIT_CAP 8

void covec_init(covec_t *vec, uint16_t itemsize) {
    vec->itemsize = itemsize;
    vec->size = 0;
    vec->cap = 0;
    vec->data = NULL;
}

void covec_free(covec_t *vec) {
    covec_clear(vec);
}

void covec_clear(covec_t *vec) {
    free(vec->data);
    vec->data = NULL;
    vec->cap = 0;
    vec->size = 0;
}

void covec_resize(covec_t *vec, int size) {
    if (vec->size == size)
        return;
    if (vec->cap >= size) {
        vec->size = size;
    } else {
        vec->data = realloc(vec->data, size*vec->itemsize);
        vec->cap = size;
        vec->size = size;
    }
}

void covec_growcap(covec_t *vec, int cap) {
    if (cap > vec->cap) {
        vec->data = realloc(vec->data, cap*vec->itemsize);
        vec->cap = cap;
    }
}

static inline void _check_and_grow(covec_t *vec) {
    if (vec->size + 1 >= vec->cap) {
        int cap = CO_MAX(INIT_CAP, vec->cap*2);
        covec_growcap(vec, cap);
    }
}

bool covec_push_at(covec_t *vec, int index, void *data) {
    if (index < 0)
        index = vec->size + index;
    if (index < 0 || index > vec->size)
        return false;
    _check_and_grow(vec);

    char *dst = (char*)vec->data;
    if (index != vec->size) {
        memmove(dst + (index + 1) * vec->itemsize, dst + index * vec->itemsize, 
            (vec->size - index) * vec->itemsize);
    }
    memcpy(dst + index * vec->itemsize, data, vec->itemsize);
    vec->size++;
    return true;
}

bool covec_push_first(covec_t *vec, void *data) {
    return covec_push_at(vec, 0, data);
}

bool covec_push_last(covec_t *vec, void *data) {
    return covec_push_at(vec, vec->size, data);
}

bool covec_get_at(covec_t *vec, int index, void *data) {
    if (index < 0)
        index = vec->size + index;
    if (index < 0 || index >= vec->size)
        return false;
    memcpy(data, (char*)vec->data + index * vec->itemsize, vec->itemsize);
    return true;
}

bool covec_get_first(covec_t *vec, void *data) {
    return covec_get_at(vec, 0, data);
}

bool covec_get_last(covec_t *vec, void *data) {
    return covec_get_at(vec, vec->size-1, data);
}

bool covec_set_at(covec_t *vec, int index, const void *data) {
    if (index < 0)
        index = vec->size + index;
    if (index < 0 || index >= vec->size)
        return false;
    memcpy((char*)vec->data + index * vec->itemsize, data, vec->itemsize);
    return true;
}

bool covec_set_first(covec_t *vec, const void *data) {
    return covec_set_at(vec, 0, data);
}

bool covec_set_last(covec_t *vec, const void *data) {
    return covec_set_at(vec, vec->size-1, data);
}

bool covec_del_at(covec_t *vec, int index, void *data) {
    if (index < 0)
        index = vec->size + index;
    if (index < 0 || index >= vec->size)
        return false;
    if (data) {
        covec_get_at(vec, index, data);
    }
    if (index != vec->size-1) {
        char *dst = (char*)vec->data;
        memmove(dst + index * vec->itemsize, dst + (index + 1) * vec->itemsize, 
            (vec->size - index - 1) * vec->itemsize);
    }
    vec->size--;
    return true;
}

bool covec_del_first(covec_t *vec, void *data) {
    return covec_del_at(vec, 0, data);
}

bool covec_del_last(covec_t *vec, void *data) {
    return covec_del_at(vec, vec->size-1, data);
}

void covec_copy(covec_t *vfrom, covec_t *vto) {
    covec_clear(vto);
    covec_resize(vto, vfrom->size);
    memcpy(vto->data, vfrom->data, vfrom->size * vfrom->itemsize);
}