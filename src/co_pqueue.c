#include "co_pqueue.h"

#define HEAP_PARENT(child) (((child) - 1) / 2)
#define HEAP_LCHILD(parent) ((parent) * 2 + 1)
#define HEAP_RCHILD(parent) ((parent) * 2 + 2)

copqueue_t* copqueue_new(uint16_t itemsize, cofn_comp_t fn, void *ud) {
    assert(fn);
    copqueue_t* pq = malloc(sizeof(*pq));
    pq->vec = covec_new(itemsize);
    pq->fn_comp = fn;
    pq->ud = ud;
    return pq;
}

void *copqueue_free(copqueue_t *pq) {
    covec_free(pq->vec);
    CO_FREE(pq);
    return NULL;
}

int copqueue_size(copqueue_t *pq) {
    return covec_size(pq->vec);
}

static void _heap_up_tail(copqueue_t *pq) {
    int size = covec_size(pq->vec);
    if (size <= 1)
        return;
    int child = size-1;
    int parent, ret;
    const void *item1, *item2;
    while (child > 0) {
        parent = HEAP_PARENT(child);
        item1 = covec_get_ptr(pq->vec, parent);
        item2 = covec_get_ptr(pq->vec, child);
        ret = pq->fn_comp(pq->ud, item1, item2);
        if (ret >= 0)
            break;
        covec_swap(pq->vec, child, parent);
        child = parent;
    }
}

static void _heap_down_head(copqueue_t *pq) {
    int size = covec_size(pq->vec);
    if (size <= 1)
        return;
    int parent = 0;
    int child = HEAP_LCHILD(parent);
    int ret, rchild;
    const void *item1, *item2;
    while (child < size) {
        // 取左右孩子较大者作为比较元素
        rchild = HEAP_RCHILD(parent);
        if (rchild < size) {
            item1 = covec_get_ptr(pq->vec, child);
            item2 = covec_get_ptr(pq->vec, rchild);
            ret = pq->fn_comp(pq->ud, item1, item2);
            if (ret < 0) {
                child = rchild;
            }
        }
        item1 = covec_get_ptr(pq->vec, parent);
        item2 = covec_get_ptr(pq->vec, child);
        ret = pq->fn_comp(pq->ud, item1, item2);
        if (ret >= 0)
            break;
        covec_swap(pq->vec, child, parent);
        parent = child;
        child = HEAP_LCHILD(parent);
    }
}

void copqueue_push(copqueue_t *pq, const void *data) {
    // 压入最后
    covec_push_tail(pq->vec, data);
    // 上浮尾元素
    _heap_up_tail(pq);
}

bool copqueue_pop(copqueue_t *pq, void *data) {
    int size = covec_size(pq->vec);
    if (size > 0) {
        // 先首尾交换
        covec_swap(pq->vec, 0, size-1);
        // 然后删除最后一个
        covec_del_tail(pq->vec, data);
        // 下沉头元素
        _heap_down_head(pq);
        return true;
    }
    return false;
}

bool copqueue_get(copqueue_t *pq, int index, void *data) {
    return covec_get(pq->vec, index, data);
}