/**
 * 定时器服务
 *                      by colin
 */
#ifndef __CO__TIMERSERVICE__
#define __CO__TIMERSERVICE__
#include "co_utils.h"
#include "co_falloc.h"
#include "co_timingwheel.h"

#ifdef __cplusplus
extern "C" {
#endif

// 定时器服务
typedef struct cots {
    cofalloc_t alloc;
    cotw_t twheel;
} cots_t;

typedef void (*fn_timer_t)(cots_t *ts, void *ud1, void *ud2, void *ud3);

// 初始化定时器服务
cots_t* cots_new(uint16_t interval, uint64_t currtime);
// 释放定时器服务
void* cots_free(cots_t* sv);
// 增加定时器
void* cots_add_timer(cots_t *sv, 
    uint32_t delay,     // 首次延迟
    uint32_t loop,      // 后续的间隔，如果为0就没有后续
    fn_timer_t cb,      // 回调函数
    void *ud1, void *ud2, void *ud3);          // 用户数据
// 删除定时器
void cots_del_timer(cots_t *sv, void **handle);
// 更新定时器
void cots_update(cots_t *sv, uint64_t currtime);


/////////////////////////////////////////////////////////////////////////////////////////////
// 调试用
void cots_print(cots_t *sv);


#ifdef __cplusplus
}
#endif
#endif