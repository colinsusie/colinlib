/**
 * 字典
 *                  by colin
 */
#ifndef __CO_DICT__
#define __CO_DICT__
#include "co_utils.h"

// 计算hash，以前判断相等性的回调
typedef uint64_t (*copfn_hash)(const void *key, size_t size);
typedef int (*copfn_equal)(const void *key1, const void *key2, size_t sz1, size_t sz2);

// 哈希结点
typedef struct codict_node {
    struct codict_node *next;       // 哈希桶的下一个结点
    struct codict_node *listprev, *listnext;    // 链表的下一个结点，遍历用
    void *key;                      // 指向key的内容
    void *value;                    // 指向value的内容
    uint64_t hash;                  // hash值
    uint16_t keysz;                 // key的大小
    uint16_t valsz;                 // value的大小
} codict_node_t;

// 字典结构
typedef struct codict {
    codict_node_t **buckets;        // 哈希桶
    codict_node_t *listhead;        // 链表头，遍历用
    codict_node_t *listtail;        // 链表尾，遍历用
    copfn_hash fn_hash;             // 哈希函数
    copfn_equal fn_equal;           // 判断相等函数
    uint32_t cap;                   // 桶位容量
    uint32_t count;                 // 结点数量
} codict_t;

// 初始化字典
void codict_init(codict_t *dict, copfn_hash fn_hash, copfn_equal fn_equal);
// 释放字典
void codict_free(codict_t *dict);
// 取结点
codict_node_t* codict_get(codict_t *dict, const void *key, size_t keysz);
// 设值
codict_node_t* codict_set(codict_t *dict, const void *key, const void *val, size_t keysz, size_t valsz);
// 删除
bool codict_del(codict_t *dict, const void *key, size_t keysz);
// 清除字典内容
void codict_clear(codict_t *dict);
// 将结点移到链表头或尾
void codict_move(codict_t *dict, codict_node_t *node, bool head);
// 字典大小
static inline size_t codict_count(codict_t *dict) { return dict->count; }
// key大小
static inline size_t codict_keysz(codict_node_t *node) { return node->keysz; }
// value大小
static inline size_t codict_valsz(codict_node_t *node) { return node->valsz; }
// 取结点的值
#define codict_value(node, type) (*(type*)((node)->value))
#define codict_value_ptr(node, type) ((type*)((node)->value))
#define codict_key(node, type) (*(type*)((node)->key))
#define codict_key_ptr(node, type) ((type*)((node)->key))

// 遍历
#define codict_begin(dict)  (dict)->listhead
#define codict_next(node)  (node)->listnext
#define codict_prev(node)  (node)->listprev
#define codict_end(dict) (dict)->listtail

////////////////////////////////////////////////////////////////////////////////
// 各种类型key的字典
// str or memory block
void codict_str(codict_t *dict);
static inline codict_node_t* codict_str_get(codict_t *dict, const char *key, size_t len) {
    return codict_get(dict, key, len);
}
static inline codict_node_t* codict_str_set(codict_t *dict, const char *key, const void *val, size_t keysz, size_t valsz) {
    return codict_set(dict, key, val, keysz, valsz);
}
static inline bool codict_str_del(codict_t *dict, const char *key, size_t len) {
    return codict_del(dict, key, len);
}

// int64
void codict_int(codict_t *dict);
static inline codict_node_t* codict_int_get(codict_t *dict, int64_t key) {
    return codict_get(dict, &key, sizeof(int64_t));
}
static inline codict_node_t* codict_int_set(codict_t *dict, int64_t key, const void *val, size_t valsz) {
    return codict_set(dict, &key, val, sizeof(int64_t), valsz);
}
static inline bool codict_int_del(codict_t *dict, int64_t key) {
    return codict_del(dict, &key, sizeof(int64_t));
}

// ptr
void codict_ptr(codict_t *dict);
static inline codict_node_t* codict_ptr_get(codict_t *dict, const void *key) {
    return codict_get(dict, &key, sizeof(void*));
}
static inline codict_node_t* codict_ptr_set(codict_t *dict, const void *key, const void *val, size_t valsz) {
    return codict_set(dict, &key, val, sizeof(void*), valsz);
}
static inline bool codict_ptr_del(codict_t *dict, const void *key) {
    return codict_del(dict, &key, sizeof(void*));
}

#endif