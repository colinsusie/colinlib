#include "../src/co_dict.h"

void test_int_dict() {

    typedef struct mydata {
        int v1;
        int v2;
    } mydata_t;

    mydata_t data1 = {10, 20};
    mydata_t data2 = {30, 40};
    mydata_t data3 = {50, 60};

    codict_t *dict = codict_new(codict_int_hash, codict_int_equal, sizeof(int), sizeof(mydata_t));

    // set data
    printf("set data======================================\n");
    codict_set_tp(dict, 1, &data1, int);
    codict_set_tp(dict, 2, &data2, int);
    codict_set_tp(dict, 3, &data3, int);
    codict_set_tp(dict, 0xFF01, &data1, int);

    // get data
    printf("get data======================================\n");
    int key = 1;
    for (key = 1; key <= 3; ++key) {
        mydata_t data = codict_value(codict_get(dict, &key), mydata_t);
        printf(">>>>>key=%d, v1=%d, v2=%d\n", key, data.v1, data.v2);
    }

    // delete
    printf("del data======================================\n");
    codict_del_tp(dict, 2, int);

    // iterate
    printf("iterate dict======================================\n");
    codict_node_t *node;
    for (node = codict_begin(dict); node != NULL; node = codict_next(node)) {
        int key = codict_key(node, int);
        mydata_t data = codict_value(node, mydata_t);
        printf(">>>>>key=%d, v1=%d, v2=%d\n", key, data.v1, data.v2);
    }

    codict_free(dict);
}


int main(int argc, char const *argv[])
{
    test_int_dict();
    return 0;
}
