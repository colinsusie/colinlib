#include "co_vec.h"

#define INIT_CAP 8

static inline void _check_and_grow(covec_t *vec) {
    if (vec->size + 1 >= vec->cap) {
        int cap = CO_MAX(INIT_CAP, vec->cap*2);
        covec_growcap(vec, cap);
    }
}

covec_t* covec_new(uint16_t itemsize) {
    covec_t *vec = CO_MALLOC(sizeof(*vec));
    vec->itemsize = itemsize;
    vec->size = 0;
    vec->cap = 0;
    vec->data = NULL;
    return vec;
}

void* covec_free(covec_t *vec) {
    covec_clear(vec);
    CO_FREE(vec);
    return NULL;
}

void covec_clear(covec_t *vec) {
    CO_FREE(vec->data);
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
        vec->data = CO_REALLOC(vec->data, size*vec->itemsize);
        vec->cap = size;
        vec->size = size;
    }
}

void covec_growcap(covec_t *vec, int cap) {
    if (cap > vec->cap) {
        vec->data = CO_REALLOC(vec->data, cap*vec->itemsize);
        vec->cap = cap;
    }
}

bool covec_push(covec_t *vec, int index, const void *data) {
    if (index < 0)
        index = vec->size + index;
    if (index < 0 || index > vec->size)
        return false;
    _check_and_grow(vec);

    if (index != vec->size) {
        covec_move(vec, index, index + 1, vec->size - index);
    }
    memcpy((char*)vec->data + index * vec->itemsize, data, vec->itemsize);
    vec->size++;
    return true;
}

bool covec_push_head(covec_t *vec, const void *data) {
    return covec_push(vec, 0, data);
}

bool covec_push_tail(covec_t *vec, const void *data) {
    return covec_push(vec, vec->size, data);
}

bool covec_get(covec_t *vec, int index, void *data) {
    if (index < 0)
        index = vec->size + index;
    if (index < 0 || index >= vec->size)
        return false;
    memcpy(data, (char*)vec->data + index * vec->itemsize, vec->itemsize);
    return true;
}

void *covec_get_ptr(covec_t *vec, int index) {
    if (index < 0)
        index = vec->size + index;
    if (index < 0 || index >= vec->size)
        return NULL;
    return (char*)vec->data + index * vec->itemsize;
}

bool covec_get_head(covec_t *vec, void *data) {
    return covec_get(vec, 0, data);
}

bool covec_get_tail(covec_t *vec, void *data) {
    return covec_get(vec, vec->size-1, data);
}

bool covec_set(covec_t *vec, int index, const void *data) {
    if (index < 0)
        index = vec->size + index;
    if (index < 0 || index >= vec->size)
        return false;
    memcpy((char*)vec->data + index * vec->itemsize, data, vec->itemsize);
    return true;
}

bool covec_set_head(covec_t *vec, const void *data) {
    return covec_set(vec, 0, data);
}

bool covec_set_tail(covec_t *vec, const void *data) {
    return covec_set(vec, vec->size-1, data);
}

bool covec_del(covec_t *vec, int index, void *data) {
    if (index < 0)
        index = vec->size + index;
    if (index < 0 || index >= vec->size)
        return false;
    if (data) {
        covec_get(vec, index, data);
    }
    if (index != vec->size-1) {
        covec_move(vec, index + 1, index, vec->size - index - 1);
    }
    vec->size--;
    return true;
}

bool covec_del_head(covec_t *vec, void *data) {
    return covec_del(vec, 0, data);
}

bool covec_del_tail(covec_t *vec, void *data) {
    return covec_del(vec, vec->size-1, data);
}

void covec_copy(covec_t *vfrom, covec_t *vto) {
    covec_clear(vto);
    covec_resize(vto, vfrom->size);
    memcpy(vto->data, vfrom->data, vfrom->size * vfrom->itemsize);
}

void covec_move(covec_t *vec, int from, int to, int count) {
    char *dst = (char*)vec->data;
    memmove(dst + to * vec->itemsize, dst + from * vec->itemsize, count * vec->itemsize);
}

void covec_swap(covec_t *vec, int idx1, int idx2) {
    const int stsize = 128;
    uint8_t data1[stsize], data2[stsize];
    uint8_t *ptr1 = vec->itemsize <= stsize ? data1 : CO_MALLOC(vec->itemsize);
    uint8_t *ptr2 = vec->itemsize <= stsize ? data2 : CO_MALLOC(vec->itemsize);
    if (covec_get(vec, idx1, ptr1) && covec_get(vec, idx2, ptr2)) {
        covec_set(vec, idx1, ptr2);
        covec_set(vec, idx2, ptr1);
    }
    if (ptr1 != data1) CO_FREE(ptr1);
    if (ptr2 != data2) CO_FREE(ptr2);
}