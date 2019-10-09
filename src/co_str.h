/**
 * 字符串函数
 *                  by colin
 */
#ifndef __CO_STR__
#define __CO_STR__
#include "co_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef char cochar;

// 字符串头
typedef struct costrheader {
    int len;        // 字符串长度, 不包括\0
    int cap;        // 字符串内存的容量
} costrheader_t;

static inline costrheader_t* costr_getheader(cochar *s) {
    return (costrheader_t*)(s - sizeof(costrheader_t*));
}

// 字符串长度
static inline int costr_len(cochar *s) {
    return costr_getheader(s)->len;
}

// 创建字符串，len如果为-1，则默认调用strlen(s)
cochar* costr_new(const char *s, int len);
// 释放
void costr_free(char *s);
// 格式化字符串，返回的字符串必须使用costr_free释放，如果有错误，返回NULL
cochar* costr_format(const char *fmt, ...);
cochar* costr_formatv(const char *fmt, va_list ap);
// TODO：
// costr_startwith
// costr_endwidth
// costr_find
// costr_replace
// costr_lower
// costr_upper
// costr_repeat

#ifdef __cplusplus
}
#endif
#endif