/**
 * epoll/kqueue 封装
 *                      by colin
 */
#ifndef __CO_EPOLL__
#define __CO_EPOLL__

#include "co_utils.h"
#include <unistd.h>
#ifdef __linux__
#   include <sys/epoll.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
#   include <sys/event.h>
#else
#   error "platform not support"
#endif

#ifdef __cplusplus
extern "C" {
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////
// 公共定义

typedef struct copollevent {
    void *ud;
	bool read;
	bool write;
	bool error;
} copollevent_t;

typedef void (*fn_poll_t)(void *copoll, copollevent_t *ev, void *ud);

////////////////////////////////////////////////////////////////////////////////////////////////////////
// linux 下使用epoll
#ifdef __linux__

typedef struct copoll {
    int efd;
    int evnum;
    struct epoll_event *events;
    void *ud;
} coepoll_t;

static inline char* copoll_name() {
    return "epoll";
}

static inline void* copoll_new() {
    coepoll_t *ep = (coepoll_t*)CO_MALLOC(sizeof(coepoll_t));
    ep->efd = epoll_create(1024);
    ep->evnum = 512;
    ep->events = (struct epoll_event*)CO_CALLOC(ep->evnum, sizeof(struct epoll_event));
    return ep;
}

static inline void* copoll_free(void *copoll) {
    coepoll_t *ep = (coepoll_t*)copoll;
    close(ep->efd);
    CO_FREE(ep->events);
    CO_FREE(ep);
    return NULL;
}

static inline void* copoll_userdata(void *copoll) {
    return ((coepoll_t*)copoll)->ud;
}

// 启动监听：默认开启读监听
static inline int copoll_add(void *copoll, int fd, void *ud) {
    coepoll_t *ep = (coepoll_t*)copoll;
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = ud;
    int ret = epoll_ctl(ep->efd, EPOLL_CTL_ADD, fd, &ev);
    return ret;
}

// 启动写监听
static inline int copoll_request_write(void *copoll, int fd, void *ud, bool enable) {
    coepoll_t *ep = (coepoll_t*)copoll;
    struct epoll_event ev;
    ev.events = EPOLLIN | (enable ? EPOLLOUT : 0);
	ev.data.ptr = ud;
    return epoll_ctl(ep->efd, EPOLL_CTL_MOD, fd, &ev);
}

// 删除监听
static inline int copoll_del(void *copoll, int fd) {
    coepoll_t *ep = (coepoll_t*)copoll;
    return epoll_ctl(ep->efd, EPOLL_CTL_DEL, fd , NULL);
}

// 等待事件到达
static inline void copoll_wait(void *copoll, int timeout, fn_poll_t fn, void *ud) {
    assert(fn);
    coepoll_t *ep = (coepoll_t*)copoll;
    int n = epoll_wait(ep->efd , ep->events, ep->evnum, timeout);
    if (n > 0) {
        int i;
        copollevent_t cpev;
        struct epoll_event *ev;
        for (i = 0; i < n; ++i) {
            ev = ep->events + i;
            cpev.ud = ev->data.ptr;
            cpev.read = (ev->events & (EPOLLIN | EPOLLHUP)) != 0;
            cpev.write = (ev->events & EPOLLOUT) != 0;
            cpev.error = (ev->events & EPOLLERR) != 0;
            fn(copoll, &cpev, ud);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BSD 下使用kqueue
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)

typedef struct cokqueue {
    int efd;
    int evnum;
    struct kevent *events;
    void *ud;
} cokqueue_t;

static inline char* copoll_name() {
    return "kqueue";
}

static inline void* copoll_new() {
    cokqueue_t *ep = (cokqueue_t*)CO_MALLOC(sizeof(cokqueue_t));
    ep->efd = kqueue();
    ep->evnum = 512;
    ep->events = CO_CALLOC(ep->evnum, sizeof(struct kevent));
    return ep;
}

static inline void* copoll_free(void *copoll) {
    cokqueue_t *ep = (cokqueue_t*)copoll;
    close(ep->efd);
    CO_FREE(ep->events);
    CO_FREE(ep);
    return NULL;
}

static inline void* copoll_userdata(void *copoll) {
    return ((cokqueue_t*)copoll)->ud;
}

// 删除监听
static inline int copoll_del(void *copoll, int fd) {
    cokqueue_t *ep = (cokqueue_t*)copoll;
    struct kevent ke;
	EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	kevent(ep->efd, &ke, 1, NULL, 0, NULL);
	EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	kevent(ep->efd, &ke, 1, NULL, 0, NULL);
    return 0;
}

// 启动读监听
static inline int copoll_add(void *copoll, int fd, void *ud) {
    cokqueue_t *ep = (cokqueue_t*)copoll;
    struct kevent ke;
	EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, ud);
	if (kevent(ep->efd, &ke, 1, NULL, 0, NULL) == -1 ||	ke.flags & EV_ERROR) {
		return -1;
	}
	EV_SET(&ke, fd, EVFILT_WRITE, EV_ADD, 0, 0, ud);
	if (kevent(ep->efd, &ke, 1, NULL, 0, NULL) == -1 ||	ke.flags & EV_ERROR) {
		EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		kevent(ep->efd, &ke, 1, NULL, 0, NULL);
		return -1;
	}
	EV_SET(&ke, fd, EVFILT_WRITE, EV_DISABLE, 0, 0, ud);
	if (kevent(ep->efd, &ke, 1, NULL, 0, NULL) == -1 ||	ke.flags & EV_ERROR) {
		copoll_del(copoll, fd);
		return -1;
	}
	return 0;
}

// 启动写监听：默认开启读监听
static inline int copoll_request_write(void *copoll, int fd, void *ud, bool enable) {
    cokqueue_t *ep = (cokqueue_t*)copoll;
    struct kevent ke;
	EV_SET(&ke, fd, EVFILT_WRITE, enable ? EV_ENABLE : EV_DISABLE, 0, 0, ud);
	if (kevent(ep->efd, &ke, 1, NULL, 0, NULL) == -1 || ke.flags & EV_ERROR)
		return -1;
	return 0;
}

// 等待事件到达
static inline void copoll_wait(void *copoll, int timeout, fn_poll_t fn, void *ud) {
    assert(fn);
    cokqueue_t *ep = (cokqueue_t*)copoll;
    struct timespec to;
    struct timespec *pto;
    if (timeout < 0) {
        pto = NULL;
    } else {
        pto = &to;
        to.tv_sec = timeout / 1000;
        to.tv_nsec = (timeout % 1000) * 1000 * 1000;
    }
    int n = kevent(ep->efd, NULL, 0, ep->events, ep->evnum, pto);
    int i;
    copollevent_t cpev;
    struct kevent *ev;
    for (i = 0; i < n; ++i) {
        ev = ep->events + i;
        cpev.ud = ev->udata;
        cpev.read = (ev->filter & EVFILT_READ) != 0;
        cpev.write = (ev->filter & EVFILT_WRITE) != 0;
        cpev.error = (ev->flags & EV_ERROR) != 0;
        fn(copoll, &cpev, ud);
    }
}

#endif

#ifdef __cplusplus
}
#endif
#endif