#include "co_timerservice.h"

typedef struct cotitem {
    cots_t *sv;
    fn_timer_t cb;
	cotnode_t node;
	void *ud1;
    void *ud2;
    void *ud3;
    uint32_t loop;
} cotitem_t;

cots_t* cots_new(uint16_t interval, uint64_t currtime) {
    cots_t* sv = CO_MALLOC(sizeof(cots_t));
    cofalloc_init(&sv->alloc, 4096, sizeof(cotitem_t));
    cotw_init(&sv->twheel, interval, currtime);
    return sv;
}

void* cots_free(cots_t* sv) {
    cofalloc_free(&sv->alloc);
    CO_FREE(sv);
    return NULL;
}

void _on_timer(void *ud) {
    cotitem_t *item = ud;
    cots_t *sv = item->sv;
    if (item->loop) {
        cotw_add(&sv->twheel, &item->node, item->loop);
    }
    if (item->cb) {
        item->cb(sv, item->ud1, item->ud2, item->ud3);
    }
}

void* cots_add_timer(cots_t *sv, uint32_t delay, uint32_t loop, fn_timer_t cb, void *ud1, void *ud2, void *ud3) {
    cotitem_t *item =  cofalloc_newitem(&sv->alloc);
    item->sv = sv;
    item->loop = loop;
    item->cb = cb;
    item->ud1 = ud1;
    item->ud2 = ud2;
    item->ud3 = ud3;
    cotw_node_init(&item->node, _on_timer, item);
    cotw_add(&sv->twheel, &item->node, delay);
    return item;
}

void cots_del_timer(cots_t *sv, void **handle) {
    if (!handle || !*handle)
        return;
    if (!cofalloc_isfree(&sv->alloc, *handle)) {
        cotitem_t *item = *handle;
        cotw_del(&sv->twheel, &item->node);
        cofalloc_freeitem(&sv->alloc, item);
        *handle = NULL;
    }
}

void cots_update(cots_t *sv, uint64_t currtime) {
    cotw_update(&sv->twheel, currtime);
}

static void _cots_print(coclink_node_t *vec, int idx, int size) {
    int i;
    printf("========================================%d\n", idx);
    for (i = 0;i < size; ++i) {
        coclink_node_t *head = vec + i;
        if (!coclink_is_empty(head)) {
            coclink_node_t *node = head->next;
            while (node != head) {
                cotnode_t *tnode = (cotnode_t*)node;
                printf("  %d: (expire=%u)\n", i, tnode->expire);
                node = node->next;
            }
        }
    }
}

void cots_print(cots_t *sv) {
    _cots_print(sv->twheel.tvroot.vec, 0, TVR_SIZE);
    _cots_print(sv->twheel.tv[0].vec, 1, TVN_SIZE);
    _cots_print(sv->twheel.tv[1].vec, 2, TVN_SIZE);
    _cots_print(sv->twheel.tv[2].vec, 3, TVN_SIZE);
    _cots_print(sv->twheel.tv[3].vec, 4, TVN_SIZE);
}