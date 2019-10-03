#include <stdio.h>
#include "../src/co_falloc.h"

typedef struct myitem {
    int v1;
    int v2;
} myitem_t;

static myitem_t* items[1000];

int main(int argc, char const *argv[])
{
    co_falloc_t alloc;
    co_falloc_init(&alloc, 0, sizeof(myitem_t));

    int i;
    int c = sizeof(items) / sizeof(items[0]);
    for (i = 0; i < c; ++i) {
        items[i] = co_falloc_newitem(&alloc);
        items[i]->v1 = i;
        items[i]->v1 = i * i;
    }

    for (i = 0; i < 500; ++i) {
        if (!co_falloc_freeitem(&alloc, items[i%c])) {
            printf("free failed: %d\n", i);
        }
    }

    for (i = 0; i < c; ++i) {
        items[i] = co_falloc_newitem(&alloc);
        items[i]->v1 = i;
        items[i]->v1 = i * i;
    }

    co_falloc_free(&alloc);
    return 0;
}
