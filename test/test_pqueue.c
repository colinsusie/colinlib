#include "../src/co_pqueue.h"

// 随机数范围[mi, ma]
int randint(int mi, int ma) {
    double r = (double)rand() * (1.0 / ((double)RAND_MAX + 1.0));
    r *= (double)(ma - mi) + 1.0;
    return (int)r + mi;
}

// 打印堆中的元素
void printq(copqueue_t *queue) {
    int i;
    for (i = 0; i < copqueue_size(queue); ++i) {
        int v;
        copqueue_get(queue, i, &v);
        printf("%d ", v);
    }
    printf("\n");
}

// 最大值在前面
static int compare1(void *ud, const void *item1, const void *item2) {
    int v1 = *((int*)item1);
    int v2 = *((int*)item2); 
    return v1 - v2;
}

// 最小值在前面
static int compare2(void *ud, const void *item1, const void *item2) {
    int v1 = *((int*)item1);
    int v2 = *((int*)item2);
    return v2 - v1;
}

void test_pqeue(int maxinum) {
    copqueue_comp_t fn = maxinum ? compare1 : compare2;
    copqueue_t *queue = copqueue_new(sizeof(int), fn, NULL);
    int i, v;
    for (i = 0; i < 11; ++i) {
        v = randint(0, 40);
        copqueue_push(queue, &v);
    }
    printq(queue);

    while (copqueue_size(queue)) {
        int v;
        if (copqueue_pop(queue, &v)) {
            printf("%d\n", v);
            printq(queue);
        }
    }
    copqueue_free(queue);
}

int main(int argc, char const *argv[])
{
    test_pqeue(1);
    test_pqeue(0);
    return 0;
}
