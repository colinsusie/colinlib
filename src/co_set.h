/**
 * 集合：由红黑树实现
 *                  by colin
 */
#include "co_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct coset_node {
    struct coset_node *parent;
    struct coset_node *left;
    struct coset_node *right;
    void *data;
    int color;
} coset_node_t;

// 比较回调
// < 0 data1 < data2
// = 0 data1 == data2
// > 0 data1 > data2
typedef int (*coset_comp_t)(void *ud, const void *data1, const void *data2);

typedef struct coset {
    coset_node_t *root;
    coset_comp_t fn_comp;
    void *ud;
    int size;
    uint32_t datasize;
} coset_t;


// 创建和翻译
coset_t* coset_new(uint16_t datasize, coset_comp_t fn, void *ud);
void* coset_free(coset_t *set);
// 集合大小
static inline int coset_size(coset_t *set) { return set->size; }
// 清除内容
void coset_clear(coset_t *set);
// 增加元素
void coset_add(coset_t *set, void *data);
// 删除元素
void coset_delete(coset_t *set, void *data);
// 判断元素是否存在
bool coset_exist(coset_t *set, void *data);
// 并集
void coset_union(coset_t *set, coset_t *set2);
// 交集
void coset_intersect(coset_t *set, coset_t *set2);
// 差集
void coset_minus(coset_t *set, coset_t *set2);
// 遍历 
coset_node_t* coset_begin(coset_t *set);
coset_node_t* coset_next(coset_node_t *curr);
coset_node_t* coset_prev(coset_node_t *curr);
coset_node_t* coset_end(coset_t *set);
// 取值
#define coset_data(node, type) (*(type*)((node)->data))
#define coset_data_ptr(node, type) ((type*)((node)->data))
// 简化操作
#define coset_add_tp(set, data, type) {type v = data; coset_add(set, &v);} while(0)
#define coset_del_tp(set, data, type) {type v = data; coset_delete(set, &v);} while(0)


// 取nil结点，调试用
coset_node_t* coset_nil();

#ifdef __cplusplus
}
#endif