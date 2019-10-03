#include "../src/co_dict.h"

void test_int_dict() {
    codict_t dict;
    codict_int(&dict);

    typedef struct mydata {
        int v1;
        int v2;
    } mydata_t;

    mydata_t data1 = {10, 20};
    mydata_t data2 = {30, 40};
    mydata_t data3 = {50, 60};

    // set data
    printf("set data======================================\n");
    codict_int_set(&dict, 1, &data1, sizeof(data1));
    codict_int_set(&dict, 2, &data2, sizeof(data2));
    codict_int_set(&dict, 3, &data3, sizeof(data3));
    codict_int_set(&dict, 0xFF01, &data3, sizeof(data3));

    // get data
    printf("get data======================================\n");
    int key = 1;
    for (key = 1; key <= 3; ++key) {
        mydata_t data = codict_value(codict_int_get(&dict, key), mydata_t);
        printf(">>>>>key=%d, v1=%d, v2=%d\n", key, data.v1, data.v2);
    }

    // delete
    printf("del data======================================\n");
    codict_int_del(&dict, 2);

    // iterate
    printf("iterate dict======================================\n");
    codict_node_t *node;
    for (node = codict_begin(&dict); node != NULL; node = codict_next(node)) {
        int64_t key = codict_key(node, int64_t);
        mydata_t data = codict_value(node, mydata_t);
        printf(">>>>>key=%lX, v1=%d, v2=%d\n", key, data.v1, data.v2);
    }

    codict_free(&dict);
}

void test_str_dict() {
    srand(time(NULL));

    codict_t dict;
    codict_str(&dict);

    printf("set data======================================\n");
    int i;
    char key[100];
    for (i = 0; i < 100; ++i) {
        sprintf(key, "key%d", i);
        size_t len = strlen(key);
        codict_str_set(&dict, key, &i, len, sizeof(int));
    }

    printf("del data======================================\n");
    for (i = 0; i < 70; ++i) {
        int idx = rand() % 100;
        sprintf(key, "key%d", idx);
        size_t len = strlen(key);
        codict_str_del(&dict, key, len);
    }

    printf("get data======================================\n");
    
    for (i = 0; i < 100; ++i) {
        sprintf(key, "key%d", i);
        size_t len = strlen(key);
        codict_node_t *node = codict_str_get(&dict, key, len);
        if (node) {
            int val = codict_value(node, int);
            printf("%s=%d\n", key, val);
            codict_move(&dict, node, true);
        }
    }

    printf("dict count=%ld\n", codict_count(&dict));

    codict_node_t *node;
    for (node = codict_begin(&dict); node != NULL; node = codict_next(node)) {
        int val = codict_value(node, int);
        printf(">>>>>v=%d\n", val);
    }

    codict_free(&dict);
}

int main(int argc, char const *argv[])
{
    test_int_dict();
    test_str_dict();
    return 0;
}
