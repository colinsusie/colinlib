#include "co_timerservice.h"

typedef struct cotitem {
    cots_t *sv;
    timer_cb_t cb;
	cotnode_t node;
	void *ud;
    uint32_t loop;
} cotitem_t;

cots_t* cots_init(uint16_t interval, uint64_t currtime) {
    cots_t* sv = CO_MALLOC(sizeof(cots_t));
    cofalloc_init(&sv->alloc, 4096, sizeof(cotitem_t));
    cotw_init(&sv->twheel, interval, currtime);
    return sv;
}

void cots_free(cots_t* sv) {
    cofalloc_free(&sv->alloc);
    CO_FREE(sv);
}

void _on_timer(void *ud) {
    cotitem_t *item = ud;
    cots_t *sv = item->sv;
    if (item->loop) {
        cotw_add(&sv->twheel, &item->node, item->loop);
    }
    if (item->cb) {
        item->cb(item->ud);
    }
}

void* cots_add_timer(cots_t *sv, uint32_t delay, uint32_t loop, timer_cb_t cb, void *ud) {
    cotitem_t *item =  cofalloc_newitem(&sv->alloc);
    item->sv = sv;
    item->loop = loop;
    item->cb = cb;
    item->ud = ud;
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