/**
 * 时间轮算法
 *                  by colin
 */
#ifndef __CO_TIMINGWHEEL__
#define __CO_TIMINGWHEEL__
#include "co_utils.h"
#include "co_clink.h"

#ifdef __cplusplus
extern "C" {
#endif

// 第1个轮占的位数
#define TVR_BITS 8
// 第1个轮的长度
#define TVR_SIZE (1 << TVR_BITS)
// 第n个轮占的位数
#define TVN_BITS 6
// 第n个轮的长度
#define TVN_SIZE (1 << TVN_BITS)
// 掩码：取模或整除用
#define TVR_MASK (TVR_SIZE - 1)
#define TVN_MASK (TVN_SIZE - 1)

// 定时器回调函数
typedef void (*timer_cb_t)(void*);

// 定时器结点
typedef struct cotnode {
    coclink_node_t *next;        // 下一个结点
    coclink_node_t *prev;        // 上一个结点
    void *userdata;               // 用户数据
    timer_cb_t callback;          // 回调函数
    uint32_t expire;              // 到期时间
} cotnode_t;

// 第1个轮
typedef struct tvroot {
    coclink_node_t vec[TVR_SIZE];
} tvroot_t;

// 后面几个轮
typedef struct tvnum {
    coclink_node_t vec[TVN_SIZE];
} tvnum_t;

// 时间轮定时器
typedef struct cotw {
    tvroot_t tvroot;               // 第1个轮
    tvnum_t tv[4];                 // 后面4个轮
    uint64_t lasttime;             // 上一次的时间毫秒
    uint32_t currtick;             // 当前的tick
    uint16_t interval;             // 每个时间点的毫秒间隔
    uint16_t remainder;            // 剩余的毫秒
} cotw_t;

// 初始化时间轮，interval为每帧的间隔，currtime为当前时间
void cotw_init(cotw_t *tw, uint16_t interval, uint64_t currtime);
// 初始化时间结点：cb为回调，ud为用户数据
void cotw_node_init(cotnode_t *node, timer_cb_t cb, void *ud);
// 增加时间结点，ticks为触发间隔(注意是以interval为单位)
void cotw_add(cotw_t *tw, cotnode_t *node, uint32_t ticks);
// 删除结点
int cotw_del(cotw_t *tw, cotnode_t *node);
// 更新时间轮
void cotw_update(cotw_t *tw, uint64_t currtime);

#ifdef __cplusplus
}
#endif
#endif