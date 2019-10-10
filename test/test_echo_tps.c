#include "../src/co_loop.h"

coloop_t *loop = NULL;
cotcp_t *client_tcp = NULL;
void * thandler = NULL;
char *buff = NULL;
int buffsize = 1024;
int64_t sendcount = 0;
int64_t recvcount = 0;
bool done = false;
uint64_t starttime = 0;

void on_close(coios_t *ss, cotcp_t* tcp) {
    printf("on_close: %d\n", tcp->fd);
    coloop_stop(ss->loop);
}

void on_error(coios_t *ss, cotcp_t* tcp, const char *msg) {
    fprintf(stderr, "on_error: %s\n", msg);
    coloop_stop(ss->loop);
}

void on_recv(coios_t *ss, cotcp_t* tcp, const void *buff, int size) {
    recvcount += size;
    if (done && recvcount == sendcount) {
        uint64_t tm = gettime();
        printf("done: %ju\n", tm - starttime);
        cotcp_close(ss, tcp, true);
        // coloop_stop(ss->loop);
    }
}

void on_connect_error(coios_t *ss, cotcp_t* tcp, const char *msg) {
    fprintf(stderr, "on_connect_error: %s\n", msg);
    coloop_stop(ss->loop);
}

void on_timer(void *ud) {
    int i;
    for (i = 0; i < 200; ++i) {
        int size = randrange(128, buffsize);
        cotcp_send(loop->ioserivce, client_tcp, buff, size);
        sendcount += size;
    }
    if (sendcount > 0xFFFFFFF) {
        done = true;
        cots_del_timer(loop->timeservice, &thandler);
    }
}

void on_connected(coios_t *ss, cotcp_t* tcp) {
    printf("Start testing, please wait...\n");
    client_tcp = tcp;
    cotcp_on_recv(ss, tcp, on_recv);
    cotcp_on_close(ss, tcp, on_close);
    cotcp_on_error(ss, tcp, on_error);

    buff = CO_MALLOC(buffsize);
    memset(buff, 'a', buffsize);
    starttime = gettime();
    thandler = cots_add_timer(ss->loop->timeservice, 1, 1, on_timer, NULL);
}

void run_test(coloop_t *loop) {
    cotcp_connect(loop->ioserivce, "127.0.0.1", "3458", NULL,  on_connected,  on_connect_error);
}

int main(int argc, char const *argv[]) {
    loop = coloop_new(1);
    run_test(loop);
    coloop_run(loop);
    coloop_free(loop);
    return 0;
}
