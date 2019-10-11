#include "../src/co_utils.h"

int main(int argc, char const *argv[])
{
    int i;
    for (i = 0; i < 100; ++i) {
        printf("%f\n", co_random());
    }
    return 0;
}
