#include "co_loop.h"


coloop_t* coloop_new(uint16_t interval) {
    coloop_t *loop = CO_MALLOC(sizeof(*loop));
    loop->timeout = interval;
    loop->currtime = co_gettime();
    loop->fn_idle = NULL;
    loop->fn_process = NULL;
    loop->stop = false;
    loop->timeservice = cots_new(interval, loop->currtime);
    loop->ioserivce = coios_new(loop);
    loop->poll = copoll_new();
    return loop;
}

void* coloop_free(coloop_t *loop) {
    cots_free(loop->timeservice);
    coios_free(loop->ioserivce);
    copoll_free(loop->poll);
    CO_FREE(loop);
    return NULL;
}

void coloop_idlecb(coloop_t *loop, fn_idle_t fn) {
    loop->fn_idle = fn;
}

void coloop_processcb(coloop_t *loop, fn_process_t fn) {
    loop->fn_process = fn;
}

void coloop_stop(coloop_t *loop) {
    loop->stop = true;
}

static void _copoll_callback(void *copoll, copollevent_t *ev, void *ud) {
    coloop_t *loop = ud;
    if (loop->fn_process) {
        loop->fn_process(loop, ev);
    }
}

void coloop_runonce(coloop_t *loop) {
    copoll_wait(loop->poll, loop->timeout, _copoll_callback, loop);
    loop->currtime = co_gettime();
    cots_update(loop->timeservice,  loop->currtime);
}

void coloop_run(coloop_t *loop) {
    while (!loop->stop) {
        coloop_runonce(loop);
        if (loop->fn_idle)
            loop->fn_idle(loop);
    }
}

