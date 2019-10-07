#include "co_timingwheel.h"

#define FIRST_INDEX(v) ((v) & TVR_MASK)
#define NTH_INDEX(v, n) (((v) >> (TVR_BITS + (n) * TVN_BITS)) & TVN_MASK)

void cotw_init(cotw_t *tw, uint16_t interval, uint64_t currtime) {
    memset(tw, 0, sizeof(*tw));
    tw->interval = interval;
    tw->lasttime = currtime;
    int i, j;
    for (i = 0; i < TVR_SIZE; ++i) {
        coclink_init(tw->tvroot.vec + i);
    }
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < TVN_SIZE; ++j)
            coclink_init(tw->tv[i].vec + j);
    }
}

void cotw_node_init(cotnode_t *node, timer_cb_t cb, void *ud) {
    node->next = 0;
    node->prev = 0;
    node->userdata = ud;
    node->callback = cb;
    node->expire = 0;
}

static void _cotw_add(cotw_t *tw, cotnode_t *node) {
    uint32_t expire = node->expire;
    uint32_t idx = expire - tw->currtick;
    coclink_node_t *head;
    if (idx < TVR_SIZE) {
        head = tw->tvroot.vec + FIRST_INDEX(expire);
    } else {
        int i;
        uint64_t sz;
        for (i = 0; i < 4; ++i) {
            sz = (1ULL << (TVR_BITS + (i+1) * TVN_BITS));
            if (idx < sz) {
                idx = NTH_INDEX(expire, i);
                head = tw->tv[i].vec + idx;
                break;
            }
        }
    }
    coclink_add_back(head, (coclink_node_t*)node);
}

void cotw_add(cotw_t *tw, cotnode_t *node, uint32_t ticks) {
    node->expire = tw->currtick + ((ticks > 0) ? ticks : 1);
    _cotw_add(tw, node);
}

int cotw_del(cotw_t *tw, cotnode_t *node) {
    if (!coclink_is_empty((coclink_node_t*)node)) {
        coclink_remote((coclink_node_t*)node);
        return 1;
    }
    return 0;
}

void _cotw_cascade(cotw_t *tw, tvnum_t *tv, int idx) {
    coclink_node_t head;
    coclink_init(&head);
    coclink_splice(tv->vec + idx, &head);
    while (!coclink_is_empty(&head)) {
        cotnode_t *node = (cotnode_t*)head.next;
        coclink_remote(head.next);
        _cotw_add(tw, node);
    }
}

void _cotw_tick(cotw_t *tw) {
    ++tw->currtick;

    uint32_t currtick = tw->currtick;
    int index = (currtick & TVR_MASK); 
    if (index == 0) {
        int i = 0;
        int idx;
        do {
            idx = NTH_INDEX(tw->currtick, i);
            _cotw_cascade(tw, &(tw->tv[i]), idx);
        } while (idx == 0 && ++i < 4);
    }

    coclink_node_t head;
    coclink_init(&head);
    coclink_splice(tw->tvroot.vec + index, &head);
    while (!coclink_is_empty(&head)) {
        cotnode_t *node = (cotnode_t*)head.next;
        coclink_remote(head.next);
        if (node->callback) {
            node->callback(node->userdata);
        }
    }
}

void cotw_update(cotw_t *tw, uint64_t currtime) {
    if (currtime > tw->lasttime) {
        int diff = currtime - tw->lasttime + tw->remainder;
        int intv = tw->interval;
        tw->lasttime = currtime;
        while (diff >= intv) {
            diff -= intv;
            _cotw_tick(tw);
        }
        tw->remainder = diff;
    }
}