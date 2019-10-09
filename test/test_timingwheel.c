#include "../src/co_timingwheel.h"
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

static cotw_t tw;
static cotnode_t node;
static cotnode_t node2;
static cotnode_t node3;
static cotnode_t node4;

void _print_timerwheel(coclink_node_t *vec, int idx, int size) {
    int i;
    printf("========================================%d\n", idx);
    for (i = 0;i < size; ++i) {
        coclink_node_t *head = vec + i;
        if (!coclink_is_empty(head)) {
            coclink_node_t *node = head->next;
            while (node != head) {
                cotnode_t *tnode = (cotnode_t*)node;
                printf("  %d: (expire=%u)\n", i, tnode->expire);
                node = node->next;
            }
        }
    }
}

void print_timerwheel() {
    _print_timerwheel(tw.tvroot.vec, 0, TVR_SIZE);
    _print_timerwheel(tw.tv[0].vec, 1, TVN_SIZE);
    _print_timerwheel(tw.tv[1].vec, 2, TVN_SIZE);
    _print_timerwheel(tw.tv[2].vec, 3, TVN_SIZE);
    _print_timerwheel(tw.tv[3].vec, 4, TVN_SIZE);
}


void on_timer(void *ud) {
    printf("on_timer: %u, data: %jd\n", tw.currtick, ((intptr_t)ud));
    cotw_del(&tw, &node2);

    cotw_node_init(&node3, on_timer, NULL);
    cotw_add(&tw, &node3, 1000);
}

void run() {
    int i = 0;
    while (1) {
        ++i;
        usleep(1000);
        cotw_update(&tw, gettime());
    }
}

int main(int argc, char const *argv[])
{
    cotw_init(&tw, 1, gettime());
    tw.currtick = 0xFFFFFFFF;

    cotw_node_init(&node, on_timer, (void*)(intptr_t)1);
    cotw_add(&tw, &node, 400);

    cotw_node_init(&node2, on_timer, NULL);
    cotw_add(&tw, &node2, 400);

    cotw_node_init(&node4, on_timer, NULL);
    cotw_add(&tw, &node4, 0xF0FFFFFF);

    print_timerwheel();
    run();
    return 0;
}
