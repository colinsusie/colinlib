#include "../src/co_list.h"

void print_list(colist_t *list) {
    colist_node_t *node;
    int v;
    for (node = colist_begin(list); node != NULL; node = node->next) {
        colist_getvalue(list, node, &v);
        printf("%d ", v);
    }
    printf("\n");
}

int main(int argc, char const *argv[])
{
    int v;
    colist_t list;
    colist_init(&list, sizeof(int));

    for (v = 1; v <= 3; v++)
        colist_push_tail(&list, &v);
    print_list(&list);
    for (v = 4; v <= 6; v++)
        colist_push_head(&list, &v);
    print_list(&list);

    v = 7;
    colist_push_at(&list, list.head, &v, true);
    v = 8;
    colist_push_at(&list, list.head, &v, false);
    v = 9;
    colist_push_at(&list, list.tail, &v, false);
    v = 10;
    colist_push_at(&list, list.tail, &v, true);
    print_list(&list);

    colist_node_t *node;
    for (node = colist_begin(&list); node != NULL; node = node->next) {
        colist_getvalue(&list, node, &v);
        if (v == 5) {
            v = 11;
            colist_push_at(&list, node, &v, true);
            colist_push_at(&list, node, &v, false);
        } else if (v == 6) {
            v = 66;
            colist_setvalue(&list, node, &v);
        }
        if (v == 3) {
            colist_pop_at(&list, node->next, NULL);
        }
    }
    print_list(&list);

    while (list.head != list.tail) {
        colist_pop_head(&list, NULL);
    }
    print_list(&list);

    colist_pop_head(&list, NULL);
    print_list(&list);

    colist_free(&list);
    return 0;
}
