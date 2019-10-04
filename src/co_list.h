/**
 * 双向链表
 *                  by colin
 */
#ifndef __CO_LIST__
#define __CO_LIST__
#include "co_utils.h"

// 链表结点
typedef struct colist_node {
    struct colist_node *prev;
    struct colist_node *next;
} colist_node_t;

// 链表结构
typedef struct colist {
    colist_node_t *head;
    colist_node_t *tail;
    uint32_t itemsize;
} colist_t;

// 初始化和释放, itemsize为结点数据的大小
void colist_init(colist_t *list, uint16_t itemsize);
void colist_free(colist_t *list);

// 压入链表
void colist_push_head(colist_t *list, const void *data);
void colist_push_tail(colist_t *list, const void *data);
void colist_push_at(colist_t *list, colist_node_t *node, const void *data, bool before);

// 弹出链表
bool colist_pop_head(colist_t *list, void *data);
bool colist_pop_tail(colist_t *list, void *data);
bool colist_pop_at(colist_t *list, colist_node_t *node, void *data);

// 遍历
static inline colist_node_t* colist_begin(colist_t *list) { return list->head; }
static inline colist_node_t* colist_end(colist_t *list) { return list->tail; }
// 取结点值
static inline void colist_getvalue(colist_t *list, colist_node_t *node, void *data) {
    memcpy(data, (char*)node + sizeof(colist_node_t), list->itemsize);
}
static inline void colist_setvalue(colist_t *list, colist_node_t *node, const void *data) {
    memcpy((char*)node + sizeof(colist_node_t), data, list->itemsize);
}
// 取结点值指针
static inline void* colist_getptr(colist_t *list, colist_node_t *node) {
    return (char*)node + sizeof(colist_node_t);
}

#endif