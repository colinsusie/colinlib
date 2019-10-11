#include "../src/co_loop.h"

cotcp_t *client_tcp = NULL;

void on_recv(coios_t *ss, cotcp_t* tcp, const void *buff, int size) {
    write(STDOUT_FILENO, buff, size);
}

void on_close(coios_t *ss, cotcp_t* tcp) {
    printf("on_close: %d\n", tcp->fd);
    coloop_stop(ss->loop);
}

void on_error(coios_t *ss, cotcp_t* tcp, const char *msg) {
    fprintf(stderr, "on_error: %s\n", msg);
    coloop_stop(ss->loop);
}

void on_connected(coios_t *ss, cotcp_t* tcp) {
    char ip[128] = {0};
    char port[32] = {0};
    coios_getpeername(tcp->fd, ip, 128, port, 32);
    printf("connected to %s:%s\n", ip, port);
    client_tcp = tcp;
    // 连接成功，监听事件
    cotcp_on_recv(ss, tcp, on_recv);
    cotcp_on_close(ss, tcp, on_close);
    cotcp_on_error(ss, tcp, on_error);
}

void on_connect_error(coios_t *ss, cotcp_t* tcp, const char *msg) {
    fprintf(stderr, "on_connect_error: %s\n", msg);
    coloop_stop(ss->loop);
}

void on_stdin_input(coios_t *ss, cofd_t *fd, const void *buf, int size) {
    // 从stdin得到数据，发送给服务器
    if (client_tcp)
        cotcp_send(ss, client_tcp, buf, size);
}

void run_client(coloop_t *loop) {
    // 绑定stdin到异步IO
    cofd_t *fd = cofd_bind(loop->ioserivce, STDIN_FILENO, NULL);
    cofd_on_recv(loop->ioserivce, fd, on_stdin_input);
    // 连接服务器
    cotcp_connect(loop->ioserivce, "127.0.0.1", "3458", NULL,  on_connected,  on_connect_error);
}

int main(int argc, char const *argv[])
{
    coios_ignsigpipe();
    coloop_t *loop = coloop_new(10);
    run_client(loop);
    coloop_run(loop);
    coloop_free(loop);
    return 0;
}
