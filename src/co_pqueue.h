/**
 * 优先队列
 *              by colin
 */
#ifndef __CO_PQUEUE__
#define __CO_PQUEUE__
#include "co_utils.h"
#include "co_vec.h"

#ifdef __cplusplus
extern "C" {
#endif

// 比较回调
// < 0 item1 < item2
// = 0 item1 == item2
// > 0 item1 > item2
typedef int (*cofn_comp_t)(void *ud, const void *item1, const void *item2);

typedef struct copqueue {
    covec_t *vec;          // 向量 
    cofn_comp_t fn_comp;   // 比较函数
    void *ud;              // 用户函数
} copqueue_t;

// 创建
copqueue_t* copqueue_new(uint16_t itemsize, cofn_comp_t fn, void *ud);
// 释放
void *copqueue_free(copqueue_t *pq);
// 队列大小
int copqueue_size(copqueue_t *pq);
// 压入元素
void copqueue_push(copqueue_t *pq, const void *data);
// 弹出元素
bool copqueue_pop(copqueue_t *pq, void *data);
// 取某个元素
bool copqueue_get(copqueue_t *pq, int index, void *data);

#ifdef __cplusplus
}
#endif
#endif