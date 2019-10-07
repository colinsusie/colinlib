/**
 * 固定长度的内存分配器
 *          by colin
*/
#ifndef __CO_FALLOC__
#define __CO_FALLOC__
#ifdef __cplusplus
extern "C" {
#endif

#include "co_utils.h"

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
typedef struct cofalloc {
    co_memblock_t *memblock;
    uint32_t blocksize;
    uint32_t itemsize;
    co_blockitem_t *freeitem;
} cofalloc_t;

// 初始化分配器: blocksize为内存块的长度，itemsize为分配项的大小
void cofalloc_init(cofalloc_t *alloc, uint32_t blocksize, uint32_t itemsize);
// 释放分配器
void cofalloc_free(cofalloc_t *alloc);
// 创建内存项
void* cofalloc_newitem(cofalloc_t *alloc);
// 释放内存项
bool cofalloc_freeitem(cofalloc_t *alloc, void *item);
// 判断内存项是否已经释放 
bool cofalloc_isfree(cofalloc_t *alloc, void *item);

#ifdef __cplusplus
}
#endif
#endif