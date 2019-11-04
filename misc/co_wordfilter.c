#include "co_wordfilter.h"
#include "../src/co_vec.h"
#include "../src/co_utf8.h"

typedef struct cowf_node {
    covec_t *chvec;         // 子结点数组
    int code;               // 字符码
    bool isend;             // 是否结束，从根到结束作为一个单词
} cowf_node_t;

typedef struct cowf {
    cowf_node_t *root;
} cowf_t;

static cowf_node_t* _cowf_new_node() {
    cowf_node_t *node = CO_MALLOC(sizeof(*node));
    node->code = 0;
    node->isend = false;
    node->chvec = NULL;
    return node;
}

static void _cowf_free_node(cowf_node_t *node) {
    if (node->chvec) {
        int i;
        int len = covec_size(node->chvec);
        for (i = 0; i < len; ++i) {
            cowf_node_t *child = covec_get_tp(node->chvec, i, cowf_node_t*);
            _cowf_free_node(child);
        }
        node->chvec = covec_free(node->chvec);
    }
    CO_FREE(node);
}

void* cowf_new() {
    cowf_t *wf = CO_MALLOC(sizeof(*wf));
    memset(wf, 0, sizeof(*wf));
    return wf;
}

void* cowf_free(void *handle) {
    cowf_clear(handle);
    CO_FREE(handle);
    return NULL;
}

void cowf_clear(void *handle) {
    cowf_t *wf = handle;
    if (wf->root) {
        _cowf_free_node(wf->root);
        wf->root = NULL;
    }
}

bool cowf_loadwords(void *handle, const char *path) {
    FILE * f = fopen(path, "r");
    if (f == NULL) return false;

    char word[256];     // 关键字最多支持这么长
    while (fgets(word, 256, f)) {
        int len = strlen(word) - 1;
        if (word[len] == '\n')
            word[len] = '\0';
        if (len > 0)
            cowf_addword(handle, word);
    }
    fclose(f);
    return true;
}

static cowf_node_t* _cowf_find_child(cowf_node_t *parent, int code, int *index) {
    if (!parent->chvec)
        parent->chvec = covec_new(sizeof(cowf_node_t*));
    // 先查找，如果找到直接返回
    int n = covec_size(parent->chvec);
    int mid = 0;
    int low = 0;
    int high = n-1;
    cowf_node_t *node = NULL;
    while (low <= high) {
        mid = (high + low) / 2;
        node = covec_get_tp(parent->chvec, mid, cowf_node_t*);
        if (code == node->code) {
            *index = mid;
            return node;
        }
        else if (code > node->code)
            low = mid + 1;
        else
            high = mid - 1;
    }
    // 找不到，返回可插入的位置
    *index = (!node || code < node->code) ? mid : mid + 1;
    return NULL;
}


static cowf_node_t* _cowf_insert_child(cowf_node_t *parent, int code) {
    int index;
    cowf_node_t*node = _cowf_find_child(parent, code, &index);
    // 如果找不到就插入到合适的位置
    if (!node) {
        node = _cowf_new_node();
        node->code = code;
        covec_push(parent->chvec, index, &node);
    }
    return node;
}

void cowf_addword(void *handle, const char *word) {
    cowf_t *wf = handle;
    if (!wf->root)
        wf->root = _cowf_new_node();

    cowf_node_t *parent = wf->root;
    cowf_node_t *child = NULL;
    int code;
    const char *p = word;
    while (*p) {
        p = coutf8_decode(p, &code);
        if (!p) {
            printf("cowf_addword: utf8 decode error: %s\n", p);
            break;
        }
        child = _cowf_insert_child(parent, code);
        parent = child;
    }
    if (child)
        child->isend = true;
}

// 取一个词word的最后子结点，如果取不到返回空
static cowf_node_t* _cowf_get_lastchild(cowf_node_t *parent, const char *word) {
    cowf_node_t *child = NULL;
    int code, index;
    const char *p = word;
    while (*p) {
        p = coutf8_decode(p, &code);
        if (!p) return NULL;

        child = _cowf_find_child(parent, code, &index);
        if (!child) return NULL;

        parent = child;
    }
    return child;
}

bool cowf_exist(void *handle, const char *word) {
    cowf_t *wf = handle;
    if (!wf->root)  return false;

    cowf_node_t *child = _cowf_get_lastchild(wf->root, word);
    return child && child->isend;
}

void cowf_delword(void *handle, const char *word) {
    // 只将词尾结点的isend设为false
    cowf_t *wf = handle;
    if (!wf->root)  return;

    cowf_node_t *child = _cowf_get_lastchild(wf->root, word);
    if (child) 
        child->isend = false;
}

// 检查是否匹配到关键字，如果没匹配到，返回NULL，如果匹配到，返回结尾指针
static const char* _cowf_check_word(cowf_node_t *parent, const char *text) {
    cowf_node_t *child = NULL;
    int code, index;
    const char *p = text;
    while (*p) {
        p = coutf8_decode(p, &code);
        if (!p) return NULL;

        child = _cowf_find_child(parent, code, &index);
        if (!child)
            return NULL;
        else if (child->isend)
            return p;
        parent = child;
    }
    return NULL;
}

bool cowf_fitler(void *handle, char *text, char ch) {
    cowf_t *wf = handle;
    if (!wf->root) return false;

    bool result = false;
    int code;
    char *p = text;
    const char *e = NULL;
    while (*p) {
        e = _cowf_check_word(wf->root, p);
        if (e) {
            while (p != e) *p++ = ch;
            result = true;
        } else {
            p = (char*)coutf8_decode(p, &code);
            if (!p) break;
        }
    }
    return result;
}

bool cowf_check(void *handle, const char *text) {
    cowf_t *wf = handle;
    if (!wf->root) return false;

    int code;
    const char *p = text;
    const char *e = NULL;
    while (*p) {
        e = _cowf_check_word(wf->root, p);
        if (e) {
            return true;
        }
        p = coutf8_decode(p, &code);
        if (!p) break;
    }
    return false;
}