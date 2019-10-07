/**
 * 协程环境
 *                      by colin
 */
#ifndef __CO_CTX__
#define __CO_CTX__
#include "co_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *regs[16];
    size_t ss_size;
    char *ss_sp;
} coctx_t;

typedef void (*coctx_func_t)(uint32_t p1, uint32_t p2);

extern int coctx_make(coctx_t *ctx, coctx_func_t fn, uint32_t p1, uint32_t p2);
extern int coctx_swap(coctx_t *octx, coctx_t *ctx);

#ifdef __cplusplus
}
#endif
#endif