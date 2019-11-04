#include "co_dict.h"

#define CODICT_INIT_SIZE 61
#define CODICT_RESIZE_RADIO 0.75

static void _check_and_resize(codict_t *dict) {
    static size_t s_bucketsizes[] = {
        61, 113, 251, 509, 1021, 2039, 4093, 8191, 16381, 
        32749, 65521, 126727, 262063, 522713, 1046429, 
        2090867, 4186067, 8363639, 16777207, 33489353,
    };
    static size_t s_bucketcount = sizeof(s_bucketsizes) / sizeof(s_bucketsizes[0]);
    if (dict->count >= dict->cap * CODICT_RESIZE_RADIO) {
        int i;
        int cap = s_bucketsizes[s_bucketcount-1];
        for (i = 0; i < s_bucketcount; ++i) {
            if (s_bucketsizes[i] > dict->cap) {
                cap = s_bucketsizes[i];
                break;
            }
        }
        dict->cap = cap;
        CO_FREE(dict->buckets);
        dict->buckets = CO_CALLOC(cap, sizeof(codict_node_t*));
        codict_node_t *curr = dict->listhead;
        while (curr) {
            size_t idx = curr->hash % cap;
            curr->next = dict->buckets[idx];
            dict->buckets[idx] = curr;
            curr = curr->listnext;
        }
    }
}

codict_t* codict_new(copfn_hash fn_hash, copfn_equal fn_equal, uint16_t keysz, uint16_t valsz) {
    assert(fn_hash);
    assert(fn_equal);
    codict_t *dict = CO_MALLOC(sizeof(*dict));
    dict->cap = 0;
    dict->buckets = NULL;
    dict->count = 0;
    dict->fn_equal = fn_equal;
    dict->fn_hash = fn_hash;
    dict->listhead = NULL;
    dict->listtail = NULL;
    dict->keysz = keysz;
    dict->valsz = valsz;
    _check_and_resize(dict);
    return dict;
}

codict_node_t * codict_get(codict_t *dict, const void *key) {
    uint64_t hash = dict->fn_hash(key);
    size_t idx = hash % dict->cap;
    codict_node_t *node = dict->buckets[idx];
    for (; node != NULL; node = node->next)
        if (node->hash == hash && dict->fn_equal(node->key, key))
            break;
    return node;
}

codict_node_t* codict_set(codict_t *dict, const void *key, const void *val) {
    codict_node_t *node = codict_get(dict, key);
    if (node) {
        memcpy(node->value, val, dict->valsz);
        return node;
    }

    _check_and_resize(dict);

    uint64_t hash = dict->fn_hash(key);
    size_t idx = hash % dict->cap;
    node = CO_MALLOC(sizeof(codict_node_t) + dict->keysz + dict->valsz);
    node->next = dict->buckets[idx];
    dict->buckets[idx] = node;
    node->hash = hash;
    node->key = ((char*)node + sizeof(codict_node_t));
    memcpy(node->key, key, dict->keysz);
    node->value = ((char*)node + sizeof(codict_node_t) + dict->keysz);
    memcpy(node->value, val, dict->valsz);

    if (!dict->listhead) {
        dict->listhead = dict->listtail = node;
        node->listprev = node->listnext = NULL;
    } else {
        dict->listtail->listnext = node;
        node->listprev = dict->listtail;
        node->listnext = NULL;
        dict->listtail = node;
    }

    dict->count++;
    return node;
}

bool codict_del(codict_t *dict, const void *key) {
    uint64_t hash = dict->fn_hash(key);
    size_t idx = hash % dict->cap;
    codict_node_t *node = dict->buckets[idx];
    codict_node_t *curr, *prev;
    for (curr = node; curr != NULL; prev = curr, curr = curr->next) {
        if (curr->hash == hash && dict->fn_equal(curr->key, key))
            break;
    }
    if (!curr)
        return false;

    if (curr == node)
        dict->buckets[idx] = curr->next;
    else
        prev->next = curr->next;

    if (dict->listhead == curr) dict->listhead = curr->listnext;
    if (dict->listtail == curr) dict->listtail = curr->listprev;
    if (curr->listprev)
        curr->listprev->listnext = curr->listnext;
    if (curr->listnext)
        curr->listnext->listprev = curr->listprev;

    CO_FREE(curr);
    dict->count--;
    return true;
}

void codict_clear(codict_t *dict) {
    size_t i;
    codict_node_t *e, *t;
    for (i = 0; i < dict->cap; ++i) {
        e = dict->buckets[i];
        while (e) {
            t = e->next;
            CO_FREE(e);
            e = t;
        }
        dict->buckets[i] = NULL;
    }
    dict->listhead = dict->listtail = NULL;
    dict->count = 0;
}

void* codict_free(codict_t *dict) {
    codict_clear(dict);
    CO_FREE(dict->buckets);
    CO_FREE(dict);
    return NULL;
}

void codict_move(codict_t *dict, codict_node_t *node, bool head) {
    if (head) {
        if (dict->listhead != node) {
            if (dict->listtail == node)
                dict->listtail = node->listprev;
            if (node->listprev)
                node->listprev->listnext = node->listnext;
            if (node->listnext)
                node->listnext->listprev = node->listprev;
            node->listprev = NULL;
            node->listnext = dict->listhead;
            dict->listhead->listprev = node;
            dict->listhead = node;
        }
    } else {
        if (dict->listtail != node) {
            if (dict->listhead == node)
                dict->listhead = node->listnext;
            if (node->listprev)
                node->listprev->listnext = node->listnext;
            if (node->listnext)
                node->listnext->listprev = node->listprev;
            node->listnext = NULL;
            node->listprev = dict->listtail;
            dict->listtail->listnext = node;
            dict->listtail = node;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// 哈希函数: fnv1a64：https://github.com/devries/fnv1a
static const uint64_t fnv64_prime = UINT64_C(1099511628211);
static const uint64_t fnv64_offset = UINT64_C(14695981039346656037);
static uint64_t codict_fnv1a64_hash(const void *key, size_t len) {
    uint8_t *pointer = (uint8_t *)key;
    uint8_t *buf_end = pointer + len;
    uint64_t hash = fnv64_offset;
    while(pointer < buf_end) {
        hash ^= (uint64_t)*pointer++;
        hash *= fnv64_prime;
    }
    return hash;
}

bool codict_str_equal(const void *key1, const void *key2) {
    const char *str1 = *(const char **)(key1);
    const char *str2 = *(const char **)(key2);
    return strcmp(str1, str2) == 0;
}

bool codict_int_equal(const void *key1, const void *key2) {
    return *(int*)key1 == *(int*)key2;
}

bool codict_ptr_equal(const void *key1, const void *key2) {
    return *(intptr_t*)key1 == *(intptr_t*)key2;
}

uint64_t codict_str_hash(const void *key) {
    const char *str = *(const char **)(key);
    int len = strlen(str);
    return codict_fnv1a64_hash(str, len);
}

uint64_t codict_int_hash(const void *key) {
    return *(int*)key;
}

uint64_t codict_ptr_hash(const void *key) {
    return *(uintptr_t*)key;
}