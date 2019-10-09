#include "../src/co_loop.h"

void on_client_close(coios_t *ss, cotcp_t* tcp) {
    printf("on_client_close: %d\n", tcp->fd);
}

void on_client_error(coios_t *ss, cotcp_t* tcp, const char *msg) {
    fprintf(stderr, "on_client_error: %s\n", msg);
}

void on_client_recv(coios_t *ss, cotcp_t* tcp, const void *buff, int size) {
    cotcp_send(ss, tcp, buff, size);
}

void on_new_client(coios_t *ss, cotcp_t* tcp) {
    printf("on_new_client: %d\n", tcp->fd);
    cotcp_on_error(ss, tcp, on_client_error);
    cotcp_on_close(ss, tcp, on_client_close);
    cotcp_on_recv(ss, tcp, on_client_recv);
}

static void run_server(coloop_t *loop) {
    char *ip = "127.0.0.1";
    char *port = "3458";
    cotcp_t * tcp = cotcp_listen(loop->ioserivce,  ip, port, NULL, on_new_client);
    if (tcp) {
        printf("listen on %s:%s, fd=%d\n", ip, port, tcp->fd);
    } else {
        fprintf(stderr, "listen failed\n");
    }
}

int main(int argc, char const *argv[])
{
    coios_ignsigpipe();
    coloop_t *loop = coloop_new(10);
    run_server(loop);
    coloop_run(loop);
    coloop_free(loop);
    return 0;
}
