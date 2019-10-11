#include "../src/co_routineex.h"

/**
 * 协程版的echo服务器 
 */

//////////////////////////////////////////////////////////////////////////
// 测试sleep
void on_test_sleep(cosched_t *sch, void *ud) {
    cortenv_t *env = ud;
    int i = 0;
    for (i = 0; i < 10; ++i) {
        cort_sleep(env, 1000);
        printf(">>>>sleep:%d\n", i);
    }
}

void test_sleep(cortenv_t *env) {
    int co = cort_new(env->sch, on_test_sleep, env);
    cort_resume(env->sch, co);
}

//////////////////////////////////////////////////////////////////////////
// 测试socket

void on_client_routine(cosched_t *sch, void *ud) {
    cotcp_t *tcp = ud;
    cortenv_t *env = tcp->ud;
    cort_tcp_bind(env, tcp);
    cobuffer_t *buff = cobuffer_new(NULL, 256, 0);
    while (true) {
        if (cort_tcp_read(env, tcp, buff)) {
            cotcp_send(tcp->ioservice, tcp, cobuffer_buffer(buff), cobuffer_pos(buff));
        } else {
            break;
        }
    }
    cobuffer_free(buff);
}

void on_new_client(coios_t *ss, cotcp_t* tcp) {
    printf("on_new_client: %d\n", tcp->fd);
    cortenv_t *env = tcp->ud;
    int co = cort_new(env->sch, on_client_routine, tcp);
    cort_resume(env->sch, co);
}

void test_socket(cortenv_t *env) {
    char *ip = "127.0.0.1";
    char *port = "3458";
    printf("listen on %s:%s\n", ip, port);
    cotcp_listen(env->loop->ioserivce,  ip, port, env, on_new_client);
}


int main(int argc, char const *argv[]) {
    coloop_t *loop = coloop_new(1);
    cortenv_t *env = cortenv_new(loop, 0);

    test_sleep(env);
    test_socket(env);
    coloop_run(loop);

    cortenv_free(env);
    coloop_free(loop);
    return 0;
}
