#include "../src/co_queue.h"

int main(int argc, char const *argv[])
{
    coqueue_t *queue = coqueue_new(sizeof(int));

    int i, v;
    for (i = 0; i < 100; ++i) {
        coqueue_push(queue, &i);
    }
    for (i = 0; i < 90; ++i) {
        coqueue_pop(queue, &v);
    }
    for (i = 0; i < 20; ++i) {
        if (coqueue_pop(queue, &v))
            printf(">>>>>>%d\n", v);
    }

    for (i = 0; i < 10; ++i) {
        coqueue_push(queue, &i);
    }
    coqueue_peek(queue, -1, &v);
    printf("peek: %d\n", v);
    coqueue_peek(queue, 0, &v);
    printf("peek: %d\n", v);

    coqueue_free(queue);
    return 0;
}
