#include "co_falloc.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define MAX_BLOCK_SIZE 16*1024
#define MIN_BLOCK_SIZE 4096
#define MIN_ITEM_SIZE sizeof(co_blockitem_t)
#define FREE_FLAG 0xC0C0C0C0

void cofalloc_init(cofalloc_t *alloc, uint32_t blocksize, uint32_t itemsize) {
    memset(alloc, 0, sizeof(*alloc));
    alloc->blocksize = CO_CLAMP(blocksize, MIN_BLOCK_SIZE, MAX_BLOCK_SIZE);
    alloc->itemsize = CO_MAX(MIN_ITEM_SIZE, itemsize); 
}

void cofalloc_free(cofalloc_t *alloc) {
    co_memblock_t *block = alloc->memblock;
    while (block) {
        co_memblock_t *temp = block->next;
        CO_FREE(block);
        block = temp;
    }
}

void* cofalloc_newitem(cofalloc_t *alloc) {
    if (!alloc->freeitem) {
        // printf("new block\n");
        co_memblock_t *block = CO_MALLOC(alloc->blocksize);
        block->next = alloc->memblock;
        alloc->memblock = block;
        int idx = 0;
        int itemsize = alloc->itemsize;
        int blocksize = alloc->blocksize;
        while (idx + itemsize <= blocksize) {
            co_blockitem_t *item = (co_blockitem_t*)(block->buffer + idx);
            item->next = alloc->freeitem;
            item->flag = 0;
            alloc->freeitem = item;
            idx += itemsize;
        }
    }
    co_blockitem_t *item = alloc->freeitem;
    alloc->freeitem = alloc->freeitem->next;
    return item;
}

bool cofalloc_freeitem(cofalloc_t *alloc, void *item) {
    co_blockitem_t *bitem = (co_blockitem_t *)item;
    if (bitem->flag == FREE_FLAG)
        return false;
    bitem->flag = FREE_FLAG;
    bitem->next = alloc->freeitem;
    alloc->freeitem = bitem;
    return true;
}

bool cofalloc_isfree(cofalloc_t *alloc, void *item) {
    co_blockitem_t *bitem = (co_blockitem_t *)item;
    return bitem->flag == FREE_FLAG;
}