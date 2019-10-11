#include "../src/co_loop.h"
#include "../src/co_dnsutils.h"

#define MAX_NAME_LEN 255
#define DEFAULT_PORT "53"
#define MAX_PACKET_LEN 512

coudp_t *udpclient = NULL;
char dns[MAX_NAME_LEN] = {0};
struct sockaddr_storage sendaddr;
socklen_t addrlen;
uint8_t sendbuf[MAX_PACKET_LEN];

void on_close(coios_t *ss, coudp_t* udp) {
    printf("on_close\n");
    udpclient = NULL;
}

void on_error(coios_t *ss, coudp_t* udp, const char *msg) {
    fprintf(stderr, "%s\n", msg);
    udpclient = NULL;
}

void on_recv(coios_t *ss, coudp_t* udp, const void *buff, int size, struct sockaddr *addr, socklen_t addrlen) {
    // 解析回应包
    ipaddr_t *ipaddrs;
    int addrnum;
    int code = codns_parse_response(buff, size, &ipaddrs, &addrnum);
    if (code != 0) {
        switch (code) {
            case 1:
                fprintf(stderr, "Format error\n"); return;
            case 2: 
                fprintf(stderr, "Server failure\n"); return;
            case 3: 
                fprintf(stderr, "Name Error\n"); return;
            case 4: 
                fprintf(stderr, "Not Implemented\n"); return;
            case 5:
                fprintf(stderr, "Refused\n"); return;
            default:
                return;
        }
    }
    // IP列表
    char sip[40];
    int i;
    for (i = 0; i < addrnum; ++i) {
        ipaddr_t *addr = &ipaddrs[i];
        if (inet_ntop(addr->af, &addr->addr, sip, 40)) {
            printf("%s\n", sip);
        }
    }

    CO_FREE(ipaddrs);
}

coudp_t *get_udp(coios_t *ss) {
    if (!udpclient) {
        udpclient = coudp_new(ss, dns, DEFAULT_PORT, NULL, false, (struct sockaddr*)&sendaddr, &addrlen);
        coudp_on_recv(ss, udpclient, on_recv);
        coudp_on_error(ss, udpclient, on_error);
        coudp_on_close(ss, udpclient, on_close);
    }
    return udpclient;
}

void on_stdin_input(coios_t *ss, cofd_t *fd, const void *buf, int size) {
    // 生成请求包
    int sz = MAX_PACKET_LEN;
    char name[MAX_NAME_LEN] = {0};
    strncpy(name, buf, CO_MIN(MAX_NAME_LEN, size-1));
    if (codns_pack_request(sendbuf, &sz, name)) {
        fprintf(stderr, "request error: %s\n", name);
        return;
    }
    coudp_t *udp = get_udp(ss);
    coudp_send(ss, udp, sendbuf, sz, (struct sockaddr*)&sendaddr, addrlen);
}

void run_client(coloop_t *loop) {
    codns_get_server(dns, MAX_NAME_LEN);

    // 绑定stdin到异步IO
    cofd_t *fd = cofd_bind(loop->ioserivce, STDIN_FILENO, NULL);
    cofd_on_recv(loop->ioserivce, fd, on_stdin_input);

    printf("input domain name: \n");
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
