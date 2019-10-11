/**
 * 基础单元，一些常用的宏和函数
 *                      by colin
 */
#ifndef __CO_UTILS__
#define __CO_UTILS__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include "co_alloc.h"

#ifdef __cplusplus
extern "C" {
#endif

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
// 取数组长度
#define CO_ARRAY_COUNT(ary) (sizeof(ary) / sizeof(ary[0]))

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

// 取当前时间，毫秒单位
static inline uint64_t co_gettime() {
    struct timeval tm;
    gettimeofday(&tm, NULL);
    return (tm.tv_sec * 1000 + tm.tv_usec / 1000);
}

// 取a到b的随机值[a, b]
static inline int co_randrange(int a, int b) {
    double d = (double)rand() / ((double)RAND_MAX + 1.0);
    d *= (double)(b - a) + 1.0;
    return (int)d + a;
}

// 取一个随机值[0, 1)
static inline double co_random() {
    return (double)rand() / ((double)RAND_MAX + 1.0);
}

#ifdef __cplusplus
}
#endif
#endif