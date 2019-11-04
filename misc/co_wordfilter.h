/**
 * 关键词过滤
 *   文本默认是utf8格式的
 *                  by colin
 */

#include "../src/co_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

// 新建句柄
void* cowf_new();
// 释放句柄
void* cowf_free(void *handle);
// 清除所有的关键词
void cowf_clear(void *handle);
// 加载关键词：path是文本路径，格式是一行一个关键词
bool cowf_loadwords(void *handle, const char *path);
// 增加关键词
void cowf_addword(void *handle, const char *word);
// 删除关键词
void cowf_delword(void *handle, const char *word);
// 判断关键词是否存在
bool cowf_exist(void *handle, const char *word);
// 检查文本是否存在关键词
bool cowf_check(void *handle, const char *text);
// 过滤文本中的关键词，ch为代替的字符
bool cowf_fitler(void *handle, char *text, char ch);

#ifdef __cplusplus
}
#endif