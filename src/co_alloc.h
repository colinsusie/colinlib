/**
 * 内存分配单元，方便后续替换
 */
#ifndef __CO_ALLOC__
#define __CO_ALLOC__
#include <stdlib.h>

#define CO_MALLOC malloc
#define CO_CALLOC calloc
#define CO_REALLOC realloc

#endif