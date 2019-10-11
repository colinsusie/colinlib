/**
 * 协程库
 *                  by colin
 */
#ifndef __CO_ROUTINE__
#define __CO_ROUTINE__
#include "co_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

// 协程执行结束
#define CO_STATUS_DEAD 0
// 协程创建后未resume，或yield后处的状态
#define CO_STATUS_SUSPEND 1
// 协程当前正在运行
#define CO_STATUS_RUNNING 2
// 当前协程resume了其他协程，此时处于这个状态
#define CO_STATUS_NORMAL 3

// 类型声明
struct cosched;
typedef struct cosched cosched_t;
typedef void (*cort_func_t)(cosched_t *, void *ud);

// 打开一个调度器，每个线程一个：stsize为栈大小，传0为默认
cosched_t* cort_open(int stsize);
// 关闭调度器
void cort_close(cosched_t *);
// 新建协程
int cort_new(cosched_t *, cort_func_t, void *ud);
// 启动协程
int cort_resume(cosched_t *, int id);
// 取协程状态
int cort_status(cosched_t *, int id);
// 取当前正在运行的协程ID
int cort_running(cosched_t *);
// 调用yield让出执行权
int cort_yield(cosched_t *);
// 当前是否是主协程
bool cort_ismain(int co);

#ifdef __cplusplus
}
#endif
#endif