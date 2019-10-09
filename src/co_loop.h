/**
 * 事件循环
 *              by colin
 */
#ifndef __CO_LOOP__
#define __CO_LOOP__
#include "co_utils.h"
#include "co_timerservice.h"
#include "co_ioservice.h"
#include "co_poll.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*fn_idle_t)(coloop_t *loop);
typedef void (*fn_process_t)(coloop_t *loop, copollevent_t *ev);

typedef struct coloop {
    void *poll;
    cots_t *timeservice;
    coios_t *ioserivce;
    fn_idle_t fn_idle;
    fn_process_t fn_process;
    uint64_t currtime;
    uint32_t timeout;
    bool stop;
} coloop_t;

coloop_t* coloop_new(uint16_t interval);
void* coloop_free(coloop_t *loop);
void coloop_idlecb(coloop_t *loop, fn_idle_t fn);
void coloop_processcb(coloop_t *loop, fn_process_t fn);
void coloop_runonce(coloop_t *loop);
void coloop_run(coloop_t *loop);
void coloop_stop(coloop_t *loop);


#ifdef __cplusplus
}
#endif
#endif