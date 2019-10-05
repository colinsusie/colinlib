#include "co_list.h"

void colist_init(colist_t *list, uint16_t itemsize) {
    list->head = NULL;
    list->tail = NULL;
    list->itemsize = itemsize;
}

void colist_free(colist_t *list) {
    colist_node_t *node = list->head;
    colist_node_t *temp;
    while (node) {
        temp = node->next;
        free(node);
        node = temp;
    }
    list->head = NULL;
    list->tail = NULL;
}

void colist_push_head(colist_t *list, const void *data) {
    colist_node_t *node = CO_MALLOC(list->itemsize);
    memset(node, 0, sizeof(colist_node_t));
    memcpy((char*)node + sizeof(colist_node_t), data, list->itemsize);
    if (!list->head) {
        list->head = list->tail = node;
    } else {
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
}

void colist_push_tail(colist_t *list, const void *data) {
    colist_node_t *node = CO_MALLOC(list->itemsize);
    memset(node, 0, sizeof(colist_node_t));
    memcpy((char*)node + sizeof(colist_node_t), data, list->itemsize);
    if (!list->tail) {
        list->head = list->tail = node;
    } else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
}

void colist_push_at(colist_t *list, colist_node_t *node, const void *data, bool before) {
    assert(node);
    colist_node_t *newnode = CO_MALLOC(list->itemsize);
    memset(newnode, 0, sizeof(colist_node_t));
    memcpy((char*)newnode + sizeof(colist_node_t), data, list->itemsize);
    if (before) {
        newnode->next = node;
        newnode->prev = node->prev;
        if (node->prev)
            node->prev->next = newnode;
        node->prev = newnode;
        if (list->head == node)
            list->head = newnode;
    } else {
        newnode->prev = node;
        newnode->next = node->next;
        if (node->next)
            node->next->prev = newnode;
        node->next = newnode;
        if (list->tail == node)
            list->tail = newnode;
    }
}

bool colist_pop_head(colist_t *list, void *data) {
    return colist_pop_at(list, list->head, data);
}

bool colist_pop_tail(colist_t *list, void *data) {
    return colist_pop_at(list, list->tail, data);
}

bool colist_pop_at(colist_t *list, colist_node_t *node, void *data) {
    if (!node)
        return false;
    if (list->head == node) {
        list->head = list->head->next;
        if (list->head)
            list->head->prev = NULL;
    }
    if (list->tail == node) {
        list->tail = list->tail->prev;
        if (list->tail)
            list->tail->next = NULL;
    }
    if (node->prev)
        node->prev->next = node->next;
    if (node->next)
        node->next->prev = node->prev;
    if (data)
        colist_getvalue(list, node, data);
    free(node);
    return true;
}
