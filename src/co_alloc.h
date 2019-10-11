/**
 * 内存分配单元，方便后续替换
 *                        by colin
 */
#ifndef __CO_ALLOC__
#define __CO_ALLOC__
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CO_MALLOC malloc
#define CO_CALLOC calloc
#define CO_REALLOC realloc
#define CO_FREE free

#ifdef __cplusplus
}
#endif
#endif