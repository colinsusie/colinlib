#include "co_set.h"

// 空结点
coset_node_t* coset_nil(coset_t *set) {
    return set->nil;
}

coset_t* coset_new(uint16_t datasize, coset_comp_t fn, void *ud) {
    assert(fn);
    coset_t *set = CO_MALLOC(sizeof(*set));
    memset(set, 0, sizeof(coset_t));
    set->nilnode.color = 1;         // 空结点是黑色的
    set->nil = &set->nilnode;       // 指向空结点
    set->root = set->nil;
    set->root->parent = set->nil;
    set->fn_comp = fn;
    set->ud = ud;
    set->datasize = datasize;
    set->size = 0;
    return set;
}

void* coset_free(coset_t *set) {
    coset_clear(set);
    CO_FREE(set);
    return NULL;
}

void coset_clear(coset_t *set) {
    coset_node_t *curr = coset_begin(set);
    while (curr) {
        coset_node_t *next = coset_next(set, curr);
        coset_delete(set, curr->data);
        curr = next;
    }
}

// 查找结点
static coset_node_t* _coset_search(coset_t *set, void *data) {
    coset_node_t *curr = set->root;
    while (curr != set->nil) {
        int ret = set->fn_comp(set->ud, data, curr->data);
        if (ret < 0)
            curr = curr->left;
        else if (ret > 0)
            curr = curr->right;
        else
            break;
    }
    return curr;
}

// 取最小结点
static coset_node_t* _coset_smallest(coset_t *set, coset_node_t *curr) {
    while (curr->left != set->nil)
        curr = curr->left;
    return curr;
}

// 取最大结点
static coset_node_t* _coset_largest(coset_t *set, coset_node_t *curr) {
    while (curr->right != set->nil)
        curr = curr->right;
    return curr;
}

// 后继
static coset_node_t* _coset_successor(coset_t *set, coset_node_t *curr) {
    // 如果右孩子存在，则找右孩子树的最小结点
    if (curr->right != set->nil)
        return _coset_smallest(set, curr->right);
    // 否则往父查找
    coset_node_t *successor = curr->parent;
    while (successor != set->nil && curr == successor->right) {
        curr = successor;
        successor = successor->parent;
    }
    return successor;
}

// 前继
static coset_node_t* _coset_predecessor(coset_t *set, coset_node_t *curr) {
    // 如果右孩子存在，则找右孩子树的最小结点
    if (curr->left != set->nil)
        return _coset_largest(set, curr->left);
    // 否则往父查找
    coset_node_t *predecessor = curr->parent;
    while (predecessor != set->nil && curr == predecessor->left) {
        curr = predecessor;
        predecessor = predecessor->parent;
    }
    return predecessor;
}

static void _coset_leftrot(coset_t *set, coset_node_t *x) {
    coset_node_t *y = x->right;         // 把y指向x的右孩子
    x->right = y->left;             // y的左孩子变成x的右孩子
    if (y->left != set->nil)     // 如果y的左孩子不为空，则设它的父为x
        y->left->parent = x;

    y->parent = x->parent;          // 更新y的父为x的父
    if (x->parent == set->nil)   // x的父为空，说明x原本是根节点，重置set->root
        set->root = y;          
    else if (x == x->parent->left) // 若原先x是其父的左孩子
        x->parent->left = y;       // 则更新后y也是其父的左孩子
    else                           // 若原先x是其父的右孩子
        x->parent->right = y;      // 则更新后y也是其父的右孩子

    y->left = x;                    // 设置y的左孩子为x
    x->parent = y;                  // 设置x的父为y，到此完成左旋
}

static void _coset_rightrot(coset_t *set, coset_node_t *y){
    coset_node_t *x = y->left;           // 把x指向y的左孩子
    y->left = x->right;              // x的右孩子变成y的左孩子
    if (x->right != set->nil)     // 如果x的右孩子不为空，则设它为父为y
        x->right->parent = y;

    x->parent = y->parent;           // 更新x的父为y的父
    if (y->parent == set->nil)    // y的父为空，说明y原本是根节点，重置set->root
        set->root = x;          
    else if (y == y->parent->left)   // 若原先y是其父的左孩子
        y->parent->left = x;         // 则更新后x也是其父的左孩子
    else                             // 若原先y是其父的右孩子
        y->parent->right = x;        // 则更新后x也是其父的右孩子

    x->right = y;                    // 设置x的右孩子为y
    y->parent = x;                   // 设置y的父为x，到此完成右旋
}

static void _coset_insert_fixedup(coset_t *set, coset_node_t *node) {
    // parent是红色才进循环
    coset_node_t *curr = node;
    while (curr->parent->color == 0) {
        if (curr->parent == curr->parent->parent->left) {   // 父是祖父的左孩子
            coset_node_t *uncle = curr->parent->parent->right;
            // case1: 若uncle是红色
            if (uncle->color == 0) {
                curr->parent->color = 1;        // 把父改成黑色
                uncle->color = 1;               // 把叔改成黑色
                curr->parent->parent->color = 0; // 把祖父改成红色
                curr = curr->parent->parent;    // 把祖父设成当前结点，继续循环
            }
            // case2 & 3: uncle是黑色
            else {  
                // case2：我是父的右孩子
                if (curr == curr->parent->right){     
                    curr = curr->parent;        // 把父设成当前结点
                    _coset_leftrot(set, curr);       // 进行左旋，左旋后就变成case3
                }
                // case3：我是父的左孩子
                curr->parent->color = 1;                      // 把父设成黑色
                curr->parent->parent->color = 0;              // 把祖父设成红色
                _coset_rightrot(set, curr->parent->parent);        // 对祖父进行右旋
            }
        }
        else {      // 父是祖父的右孩子：与上面处理相反
            coset_node_t *uncle = curr->parent->parent->left;
            // case1: 若uncle是红色
            if (uncle->color == 0) {
                curr->parent->color = 1;
                uncle->color = 1;
                curr->parent->parent->color = 0;
                curr = curr->parent->parent;
            }
            // case2 & 3: uncle是黑色
            else {
                // case2
                if (curr == curr->parent->left) {
                    curr = curr->parent;
                    _coset_rightrot(set, curr);
                }
                // case3
                curr->parent->color = 1;
                curr->parent->parent->color = 0;
                _coset_leftrot(set, curr->parent->parent);
            }
        }
    }
    // 确保父结点为黑色
    set->root->color = 1;
}


void coset_add(coset_t *set, void *data) {
    coset_node_t *parent = set->nil;
    coset_node_t *curr = set->root;
    int ret = 0;
    // 找到插入的位置
    while (curr != set->nil) {
        parent = curr;
        ret = set->fn_comp(set->ud, data, curr->data);
        if (ret < 0)
            curr = curr->left;
        else if (ret > 0)
            curr = curr->right;
        else
            return;     // 值相关不用插入，直接退出
    }
    // 创建结点
    coset_node_t *node = CO_MALLOC(sizeof(coset_node_t) + set->datasize);
    node->parent = parent;
    node->left = set->nil;
    node->right = set->nil;
    node->color = 0;
    node->data = (void*)(node + 1);
    memcpy(node->data, data, set->datasize);
    set->size++;

    // 加入父结点的树
    if (parent == set->nil)
        set->root = node;
    else if (ret < 0)
        parent->left = node;
    else
        parent->right = node;
    // 向上修正
    _coset_insert_fixedup(set, node);
}

static void _coset_delete_fixedup(coset_t *set, coset_node_t *curr) {
    while (curr != set->root && curr->color == 1) {
        if (curr == curr->parent->left) {    
            coset_node_t *sibling = curr->parent->right;
            if (sibling->color == 0) {
                sibling->color = 1;
                curr->parent->color = 0;
                _coset_leftrot(set, curr->parent);
                sibling = curr->parent->right;
            }
            if (sibling->left->color == 1 && sibling->right->color == 1) {
                sibling->color = 0;
                curr = curr->parent;
            }
            else {
                if (sibling->right->color == 1){
                    sibling->left->color = 1;
                    sibling->color = 0;
                    _coset_rightrot(set, sibling);
                    sibling = curr->parent->right;
                }
                sibling->color = curr->parent->color;
                curr->parent->color = 1;
                sibling->right->color = 1;
                _coset_leftrot(set, curr->parent);
                curr = set->root;
            }
        }
        else {  
            coset_node_t *sibling = curr->parent->left;
            if (sibling->color == 0) {
                sibling->color = 1;
                curr->parent->color = 0;
                _coset_rightrot(set, curr->parent);
                sibling = curr->parent->left;
            }
            if (sibling->left->color == 1 && sibling->right->color == 1) {
                sibling->color = 0;
                curr = curr->parent;
            }
            else {
                if (sibling->left->color == 1){
                    sibling->right->color = 1;
                    sibling->color = 0;
                    _coset_leftrot(set, sibling);
                    sibling = curr->parent->left;
                }
                sibling->color = curr->parent->color;
                curr->parent->color = 1;
                sibling->left->color = 1;
                _coset_rightrot(set, curr->parent);
                curr = set->root;
            }
        }
    }
    curr->color = 1;
}

void coset_delete(coset_t *set, void *data) {
    // 先查找结点
    coset_node_t *curr = _coset_search(set, data);
    if (curr == set->nil)
        return;

    coset_node_t *dnode = NULL;     // 要被删除的结点
    coset_node_t *child = NULL;     // 要被刪除的结点的孩子

    if (curr->left == set->nil || curr->right == set->nil)
        dnode = curr;                                // 如果最多只有一个孩子，则当前结点是删除结点
    else
        dnode = _coset_smallest(set, curr->right);  // 否则找到后继结点，作为删除结点，因为right一定存在，所以简化为找right的最小结点

    // 经过上面简化后，dnode最多只有一个孩子，将删除结点的孩子，和删除结点的父关联起来，这样删除结点就可以安全删除
    child = dnode->left != set->nil ? dnode->left : dnode->right;
    child->parent = dnode->parent;      // child有可能为NIL       
    if (dnode->parent == set->nil)
        set->root = child;
    else if (dnode == dnode->parent->left)
        dnode->parent->left = child;
    else
        dnode->parent->right = child;

    // 如果删除结点不是当前结点，就要更新当前结点的值
    if (dnode != curr)
        memcpy(curr->data, dnode->data, set->datasize);

    // 释放内存
    set->size--;

    // 如果删除结点是黑色，就可能破坏红黑树的规则，要进行修正
    if (dnode->color == 1)
        _coset_delete_fixedup(set, child);

    CO_FREE(dnode);
}

bool coset_exist(coset_t *set, void *data) {
    return _coset_search(set, data) != set->nil;
}

void coset_union(coset_t *set, coset_t *set2) {
    assert(set->datasize == set2->datasize);
    coset_node_t *curr = coset_begin(set2);
    while (curr) {
        coset_add(set, curr->data);
        curr = coset_next(set2, curr);
    }
}

void coset_intersect(coset_t *set, coset_t *set2) {
    assert(set->datasize == set2->datasize);
    coset_node_t *curr = coset_begin(set);
    while (curr) {
        coset_node_t *next = coset_next(set, curr);
        if (!coset_exist(set2, curr->data))
            coset_delete(set, curr->data);
        curr = next;
    }
}

void coset_minus(coset_t *set, coset_t *set2) {
    assert(set->datasize == set2->datasize);
    coset_node_t *curr = coset_begin(set2);
    while (curr) {
        coset_delete(set, curr->data);
        curr = coset_next(set2, curr);
    }
}

coset_node_t* coset_begin(coset_t *set) {
    if (set->root != set->nil) {
        coset_node_t *curr = _coset_smallest(set, set->root);
        return curr == set->nil ? NULL : curr;
    }
    return NULL;
}

coset_node_t* coset_next(coset_t *set, coset_node_t *curr) {
    curr = _coset_successor(set, curr);
    return curr == set->nil ? NULL : curr;
}

coset_node_t* coset_prev(coset_t *set, coset_node_t *curr) {
    curr = _coset_predecessor(set, curr);
    return curr == set->nil ? NULL : curr;
}

coset_node_t* coset_end(coset_t *set) {
    if (set->root != set->nil) {
        coset_node_t *curr = _coset_largest(set, set->root);
        return curr == set->nil ? NULL : curr;
    }
    return NULL;
}
