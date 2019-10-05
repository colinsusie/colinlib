/**
 * utf-8函数
 *              by colin
 */
#ifndef __CO_UTF8__
#define __CO_UTF8__
#include "co_utils.h"

// utf8解码，如果格式错误返回NULL；code为返回的码点，可以为NULL
const char *coutf8_decode(const char *s, int *code);
// 计算一个utf8字符串的长度
int coutf8_len(const char *s);

#endif