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

// 初始化定时器服务
cots_t* cots_init(uint16_t interval, uint64_t currtime);
// 释放定时器服务
void* cots_free(cots_t* sv);
// 增加定时器
void* cots_add_timer(cots_t *sv, 
    uint32_t delay,     // 首次延迟
    uint32_t loop,      // 后续的间隔，如果为0就没有后续
    timer_cb_t cb,      // 回调函数
    void *ud);          // 用户数据
// 删除定时器
void cots_del_timer(cots_t *sv, void **handle);
// 更新定时器
void cots_update(cots_t *sv, uint64_t currtime);

#ifdef __cplusplus
}
#endif
#endif