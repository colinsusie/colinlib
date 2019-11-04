/**
 * 字典
 *                  by colin
 */
#ifndef __CO_DICT__
#define __CO_DICT__
#include "co_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

// 计算hash，以前判断相等性的回调
typedef uint64_t (*copfn_hash)(const void *key);
typedef bool (*copfn_equal)(const void *key1, const void *key2);

// 哈希结点
typedef struct codict_node {
    struct codict_node *next;       // 哈希桶的下一个结点
    struct codict_node *listprev, *listnext;    // 链表的下一个结点，遍历用
    void *key;                      // 指向key的内容
    void *value;                    // 指向value的内容
    uint64_t hash;                  // hash值
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
    uint16_t keysz;                 // key的大小
    uint16_t valsz;                 // value的大小
} codict_t;

// 初始化字典
codict_t* codict_new(copfn_hash fn_hash, copfn_equal fn_equal, uint16_t keysz, uint16_t valsz);
// 释放字典
void* codict_free(codict_t *dict);
// 取结点
codict_node_t* codict_get(codict_t *dict, const void *key);
// 设值
codict_node_t* codict_set(codict_t *dict, const void *key, const void *val);
// 删除
bool codict_del(codict_t *dict, const void *key);
// 清除字典内容
void codict_clear(codict_t *dict);
// 将结点移到链表头或尾
void codict_move(codict_t *dict, codict_node_t *node, bool head);
// 字典大小
static inline size_t codict_count(codict_t *dict) { return dict->count; }
// 取结点的值
#define codict_value(node, type) (*(type*)((node)->value))
#define codict_value_ptr(node, type) ((type*)((node)->value))
#define codict_key(node, type) (*(type*)((node)->key))
#define codict_key_ptr(node, type) ((type*)((node)->key))

// 简化操作：key为基础类型的立即数
#define codict_get_tp(dict, key, type) {type k = key; codict_get(dict, &k);} while(0)
#define codict_set_tp(dict, key, val, type) {type k = key; codict_set(dict, &k, val);} while(0)
#define codict_del_tp(dict, key, type) {type k = key; codict_del(dict, &k);} while(0)

// 遍历
static inline codict_node_t* codict_begin(codict_t *dict) { return dict->listhead; }
static inline codict_node_t* codict_end(codict_t *dict) { return dict->listtail; }
static inline codict_node_t* codict_next(codict_node_t *node) { return node->listnext; }
static inline codict_node_t* codict_prev(codict_node_t *node) { return node->listprev; }

// 一些常用类型的回函调数，字符串必须0结尾，否则请自行提供
bool codict_str_equal(const void *key1, const void *key2);
bool codict_int_equal(const void *key1, const void *key2);
bool codict_ptr_equal(const void *key1, const void *key2);
uint64_t codict_str_hash(const void *key);
uint64_t codict_int_hash(const void *key);
uint64_t codict_ptr_hash(const void *key);

#ifdef __cplusplus
}
#endif

#endif