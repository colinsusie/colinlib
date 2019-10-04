#ifndef __CO_UTILS__
#define __CO_UTILS__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include "co_endian.h"

// 取最小值
#define CO_MIN(a, b) ((a) > (b) ? (b) : (a))
// 取最大值
#define CO_MAX(a, b) ((a) > (b) ? (a) : (b))
// 把V限定在[mi, ma]
#define CO_CLAMP(v, mi, ma) CO_MAX(CO_MIN(v, ma), mi)
// 取结构成员的偏移
#define CO_OFFSETOF(type, member) ((size_t)&((type*)0)->member)
// 通过结构的成员指针，向上取向结构指针
#define CO_CONTAINEROF(ptr, type, member) (type*)(((char*)((type*)ptr)) - CO_OFFSETOF(type, member)))

// 取下一个2的幂
static inline uint32_t roundup_pow2(uint32_t size) {
    --size;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    return ++size;
}

// 判断是否是小端字节序
static inline bool is_little() {
    static const union {
        int dummy;
        char little;
    } host_endian = {1};
    return host_endian.little;
}

#endif