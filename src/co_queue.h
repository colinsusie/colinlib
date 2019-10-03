/**
 * 先进先出队列，基于环形队列
 *                          by colin
 */
#ifndef __CO_QUEUE__
#define __CO_QUEUE__
#include "co_utils.h"
#include "co_vec.h"

// 队列结构
typedef struct coqueue {
    covec_t vec;        // 向量
    int head;           // 头
    int tail;           // 尾
} coqueue_t;

// 初始化队列，itemsize为元素大小
void coqueue_init(coqueue_t *queue, uint16_t itemsize);
// 释放队列
void coqueue_free(coqueue_t *queue);
// 队列大小
int coqueue_size(coqueue_t *queue);
// 压入元素
void coqueue_push(coqueue_t *queue, const void *data);
// 弹出元素
bool coqueue_pop(coqueue_t *queue, void *data);
// 取某个元素，0表示第1个，-1表示最后一个
bool coqueue_peek(coqueue_t *queue, int index, void *data);

#endif