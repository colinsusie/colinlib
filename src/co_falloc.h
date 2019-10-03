#ifndef __CO_FALLOC__
#define __CO_FALLOC__
/**
 * 固定长度的内存分配器
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// 内存项
typedef struct co_blockitem {
    struct co_blockitem *next;
    uint32_t flag;
    uint32_t dummy;
} co_blockitem_t;

// 内存块
typedef struct co_memblock {
    struct co_memblock *next;
    char buffer[0];
} co_memblock_t;

// 分配器
typedef struct co_falloc {
    co_memblock_t *memblock;
    uint32_t blocksize;
    uint32_t itemsize;
    co_blockitem_t *freeitem;
} co_falloc_t;

// 初始化分配器: blocksize为内存块的长度，itemsize为分配项的大小
void co_falloc_init(co_falloc_t *alloc, uint32_t blocksize, uint32_t itemsize);
// 释放分配器
void co_falloc_free(co_falloc_t *alloc);
// 创建内存项
void* co_falloc_newitem(co_falloc_t *alloc);
// 释放内存项
bool co_falloc_freeitem(co_falloc_t *alloc, void *item);
// 判断内存项是否已经释放 
bool co_falloc_isfree(co_falloc_t *alloc, void *item);

#endif