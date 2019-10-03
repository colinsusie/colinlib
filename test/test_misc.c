#include "../src/co_utils.h"

#define co_memcpy(dist, src, size)  \
    do {\
        switch (size) {\
            case 1: *(uint8_t*)(dist)= *(uint8_t*)(src); break;\
            case 2: *(uint16_t*)(dist)= *(uint16_t*)(src); break;\
            case 4: *(uint32_t*)(dist)= *(uint32_t*)(src); break;\
            case 8: *(uint64_t*)(dist)= *(uint64_t*)(src); break;\
        }\
    } while(0)


int main(int argc, char const *argv[])
{
    uint64_t d;
    uint64_t s = 0xFF00EE00;

    struct timespec ts1, ts2;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts1);

    uint64_t v = 0;
    int i;
    for (i = 0; i < 100000000; ++i) {
        co_memcpy(&d, &s, sizeof(uint64_t));
        // memcpy(&d, &s, sizeof(uint64_t));
        v += d;
    }
    
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts2);

    double tm = (ts2.tv_sec-ts1.tv_sec) * 1000.0 + (ts2.tv_nsec-ts1.tv_nsec)/1000000.0;
    printf("valur=%lu, time=%f\n", v, tm);
    return 0;
}
