#include <stdio.h>
#include "../src/co_falloc.h"

typedef struct myitem {
    int v1;
    int v2;
} myitem_t;

static myitem_t* items[1000];

int main(int argc, char const *argv[])
{
    cofalloc_t alloc;
    cofalloc_init(&alloc, 0, sizeof(myitem_t));

    int i;
    int c = sizeof(items) / sizeof(items[0]);
    for (i = 0; i < c; ++i) {
        items[i] = cofalloc_newitem(&alloc);
        items[i]->v1 = i;
        items[i]->v1 = i * i;
    }

    for (i = 0; i < 500; ++i) {
        if (!cofalloc_freeitem(&alloc, items[i%c])) {
            printf("CO_FREE failed: %d\n", i);
        }
    }

    for (i = 0; i < c; ++i) {
        items[i] = cofalloc_newitem(&alloc);
        items[i]->v1 = i;
        items[i]->v1 = i * i;
    }

    cofalloc_free(&alloc);
    return 0;
}
