#include "co_routineex.h"

cortenv_t* cortenv_new(coloop_t *loop, int stsize) {
    cortenv_t *env = CO_MALLOC(sizeof(cortenv_t));
    env->loop = loop;
    env->sch = cort_open(stsize);
    return env;
}

void* cortenv_free(cortenv_t *env) {
    cort_close(env->sch);
    CO_FREE(env);
    return NULL;
}

static void _on_sleep_timer(cots_t *ts, void *ud1, void *ud2, void *ud3) {
    cortenv_t *env = (cortenv_t*)ud1;
    int co = (int)(intptr_t)ud2;
    cort_resume(env->sch, co);
}

//////////////////////////////////////////////////////////////////////////
// tcp

void cort_sleep(cortenv_t *env, uint32_t ms) {
    int co = cort_running(env->sch);
    if (cort_ismain(co)) {
        fprintf(stderr, "The main coroutine can't sleep\n");
        return;
    }
    cots_add_timer(env->loop->timeservice, ms, 0, _on_sleep_timer, env, (void*)(intptr_t)co, NULL);
    cort_yield(env->sch);
}

typedef struct tcpreaddata {
    bool error;
    bool close;
    int co;
    cobuffer_t *buff;
    cortenv_t *env;
} tcpreaddata_t;

void on_tcp_close(coios_t *ss, cotcp_t* tcp) {
    printf("on_tcp_close: %d\n", tcp->fd);
    tcpreaddata_t *data = (tcpreaddata_t*)tcp->ud;
    data->close = true;
    data->error = false;
    cort_resume(data->env->sch, data->co); 
}

void on_tcp_error(coios_t *ss, cotcp_t* tcp, const char *msg) {
    fprintf(stderr, "on_tcp_error: %s\n", msg);
    tcpreaddata_t *data = (tcpreaddata_t*)tcp->ud;
    data->close = false;
    data->error = true;
    cort_resume(data->env->sch, data->co); 
}

void on_tcp_recv(coios_t *ss, cotcp_t* tcp, const void *buff, int size) {
    tcpreaddata_t *data = (tcpreaddata_t*)tcp->ud;
    data->close = false;
    data->error = false;
    cobuffer_write(data->buff, buff, size);
    cort_resume(data->env->sch, data->co); 
}

void cort_tcp_bind(cortenv_t *env, cotcp_t *tcp) {
    int co = cort_running(env->sch);
    if (cort_ismain(co)) {
        fprintf(stderr, "The main coroutine can't call cort_tcp_bind\n");
        return;
    }
    cotcp_on_error(env->loop->ioserivce, tcp, on_tcp_error);
    cotcp_on_close(env->loop->ioserivce, tcp, on_tcp_close);
    cotcp_on_recv(env->loop->ioserivce, tcp, on_tcp_recv);
}

bool cort_tcp_read(cortenv_t *env, cotcp_t *tcp, cobuffer_t *buff) {
    int co = cort_running(env->sch);
    if (cort_ismain(co)) {
        fprintf(stderr, "The main coroutine can't call cort_tcp_read\n");
        return false;
    }
    
    cobuffer_seek(buff, true, 0);
    tcpreaddata_t data = {false, false, co, buff, env};
    tcp->ud = &data;
    cort_yield(env->sch);

    if (data.close || data.error)
        return false;
    else
        return true;
}

typedef struct tcpconndata {
    int co;
    cotcp_t *tcp;
    cortenv_t *env;
} tcpconndata_t;

static void on_tcp_connected(coios_t *ss, cotcp_t* tcp) {
    printf("connected succed\n");
    tcpconndata_t *data = tcp->ud;
    data->tcp = tcp;
    cort_resume(data->env->sch, data->co); 
}

static void on_tcp_connect_error(coios_t *ss, cotcp_t* tcp, const char *msg) {
    fprintf(stderr, "on_connect_error: %s\n", msg);
    tcpconndata_t *data = tcp->ud;
    cort_resume(data->env->sch, data->co); 
}

cotcp_t* cort_tcp_connect(cortenv_t *env, const char *ip, const char *port) {
    int co = cort_running(env->sch);
    if (cort_ismain(co)) {
        fprintf(stderr, "The main coroutine can't call cort_tcp_read\n");
        return false;
    }

    tcpconndata_t data = {co, NULL, env};
    cotcp_connect(env->loop->ioserivce, ip, port, &data,  on_tcp_connected,  on_tcp_connect_error);
    cort_yield(env->sch);
    return data.tcp;
}

//////////////////////////////////////////////////////////////////////////
// udp

typedef struct udpreaddata {
    bool error;
    bool close;
    int co;
    cobuffer_t *buff;
    cortenv_t *env;
} udpreaddata_t;

void on_udp_close(coios_t *ss, coudp_t* udp) {
    printf("on_udp_close: %d\n", udp->fd);
    udpreaddata_t *data = (udpreaddata_t*)udp->ud;
    data->close = true;
    data->error = false;
    cort_resume(data->env->sch, data->co); 
}

void on_udp_error(coios_t *ss, coudp_t* udp, const char *msg) {
    fprintf(stderr, "on_udp_error: %s\n", msg);
    udpreaddata_t *data = (udpreaddata_t*)udp->ud;
    data->close = false;
    data->error = true;
    cort_resume(data->env->sch, data->co); 
}

void on_udp_recv(coios_t *ss, coudp_t* udp, const void *buff, int size, struct sockaddr *addr, socklen_t addrlen) {
    udpreaddata_t *data = (udpreaddata_t*)udp->ud;
    data->close = false;
    data->error = false;
    cobuffer_write(data->buff, buff, size);
    cort_resume(data->env->sch, data->co); 
}

void cort_udp_bind(cortenv_t *env, coudp_t *udp) {
    int co = cort_running(env->sch);
    if (cort_ismain(co)) {
        fprintf(stderr, "The main coroutine can't call cort_tcp_bind\n");
        return;
    }
    coudp_on_error(env->loop->ioserivce, udp, on_udp_error);
    coudp_on_close(env->loop->ioserivce, udp, on_udp_close);
    coudp_on_recv(env->loop->ioserivce, udp, on_udp_recv);
}

bool cort_udp_read(cortenv_t *env, coudp_t *udp, cobuffer_t *buff) {
    int co = cort_running(env->sch);
    if (cort_ismain(co)) {
        fprintf(stderr, "The main coroutine can't call cort_fd_read\n");
        return false;
    }
    
    cobuffer_seek(buff, true, 0);
    udpreaddata_t data = {false, false, co, buff, env};
    udp->ud = &data;
    cort_yield(env->sch);

    if (data.close || data.error)
        return false;
    else
        return true;
}

//////////////////////////////////////////////////////////////////////////
// fd

typedef struct fdreaddata {
    bool error;
    bool close;
    int co;
    cobuffer_t *buff;
    cortenv_t *env;
} fdreaddata_t;

void on_fd_close(coios_t *ss, cofd_t* fd) {
    printf("on_fd_close: %d\n", fd->fd);
    fdreaddata_t *data = (fdreaddata_t*)fd->ud;
    data->close = true;
    data->error = false;
    cort_resume(data->env->sch, data->co); 
}

void on_fd_error(coios_t *ss, cofd_t* fd, const char *msg) {
    fprintf(stderr, "on_fd_error: %s\n", msg);
    fdreaddata_t *data = (fdreaddata_t*)fd->ud;
    data->close = false;
    data->error = true;
    cort_resume(data->env->sch, data->co); 
}

void on_fd_recv(coios_t *ss, cofd_t* fd, const void *buff, int size) {
    fdreaddata_t *data = (fdreaddata_t*)fd->ud;
    data->close = false;
    data->error = false;
    cobuffer_write(data->buff, buff, size);
    cort_resume(data->env->sch, data->co); 
}


void cort_fd_bind(cortenv_t *env, cofd_t *fd) {
    int co = cort_running(env->sch);
    if (cort_ismain(co)) {
        fprintf(stderr, "The main coroutine can't call cort_tcp_bind\n");
        return;
    }
    cofd_on_error(env->loop->ioserivce, fd, on_fd_error);
    cofd_on_close(env->loop->ioserivce, fd, on_fd_close);
    cofd_on_recv(env->loop->ioserivce, fd, on_fd_recv);
}

bool cort_fd_read(cortenv_t *env, cofd_t *fd, cobuffer_t *buff) {
    int co = cort_running(env->sch);
    if (cort_ismain(co)) {
        fprintf(stderr, "The main coroutine can't call cort_fd_read\n");
        return false;
    }
    
    cobuffer_seek(buff, true, 0);
    fdreaddata_t data = {false, false, co, buff, env};
    fd->ud = &data;
    cort_yield(env->sch);

    if (data.close || data.error)
        return false;
    else
        return true;
}
