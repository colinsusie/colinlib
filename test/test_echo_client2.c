#include "../src/co_routineex.h"

/**
 * 协程版的echo客户端
 */

cotcp_t *client_tcp = NULL;

void client_rountine(cosched_t *sch, void *ud) {
    cortenv_t *env = ud;
    char *ip = "127.0.0.1";
    char *port = "3458";
    printf("connect to %s:%s\n", ip, port);
    cotcp_t *tcp = cort_tcp_connect(env, ip, port);
    if (!tcp) {
        coloop_stop(env->loop);
        return;
    }
    cort_tcp_bind(env, tcp);
    client_tcp = tcp;

    cobuffer_t *buff = cobuffer_new(NULL, 256, 0);
    while (true) {
        if (cort_tcp_read(env, tcp, buff)) {
            write(STDOUT_FILENO, cobuffer_buffer(buff), cobuffer_pos(buff));
        } else {
            break;
        }
    }
    cobuffer_free(buff);
}

void io_rountine(cosched_t *sch, void *ud) {
    cortenv_t *env = ud;
    cofd_t *fd = cofd_bind(env->loop->ioserivce, STDIN_FILENO, NULL);
    cort_fd_bind(env, fd);

    cobuffer_t *buff = cobuffer_new(NULL, 256, 0);
    while (true) {
        if (cort_fd_read(env, fd, buff) && client_tcp) {
            cotcp_send(env->loop->ioserivce, client_tcp, cobuffer_buffer(buff), cobuffer_pos(buff));
        } else {
            break;
        }
    }
    cobuffer_free(buff);
}

void test_client(cortenv_t *env) {
    int co = cort_new(env->sch, client_rountine, env);
    cort_resume(env->sch, co);
    co = cort_new(env->sch, io_rountine, env);
    cort_resume(env->sch, co);
}


int main(int argc, char const *argv[])
{
    coloop_t *loop = coloop_new(1);
    cortenv_t *env = cortenv_new(loop, 0);

    test_client(env);
    coloop_run(loop);

    cortenv_free(env);
    coloop_free(loop);
    return 0;
}
