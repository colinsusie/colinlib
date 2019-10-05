#include "../src/co_utils.h"

int main(int argc, char const *argv[])
{
    int64_t i = INT64_C(0xFF212132EDfd323);
    uint32_t b = 3332;
    printf("%jd, %u\n", i, b);
    return 0;
}
