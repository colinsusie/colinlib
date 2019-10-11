#include "../src/co_routineex.h"
#include "../src/co_dnsutils.h"

#define MAX_NAME_LEN 255
#define DEFAULT_PORT "53"
#define MAX_PACKET_LEN 512

coudp_t *udpclient = NULL;
char dns[MAX_NAME_LEN] = {0};
struct sockaddr_storage sendaddr;
socklen_t addrlen;
uint8_t sendbuf[MAX_PACKET_LEN];

static void parse_dns_res(cobuffer_t *buff) {
    ipaddr_t *ipaddrs;
    int addrnum;
    int code = codns_parse_response(buff->buffer, buff->pos, &ipaddrs, &addrnum);
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

static void udp_rountine(cosched_t *sch, void *ud) {
    cortenv_t *env = ud;
    codns_get_server(dns, MAX_NAME_LEN);
    udpclient = coudp_new(env->loop->ioserivce, dns, DEFAULT_PORT, NULL, false, 
        (struct sockaddr*)&sendaddr, &addrlen);
    if (!udpclient) {
        fprintf(stderr, "coudp_new failed\n");
        return;
    }
    cort_udp_bind(env, udpclient);
    cobuffer_t *buff = cobuffer_new(NULL, 256, 0);
    while (true) {
        if (cort_udp_read(env, udpclient, buff)) {
            parse_dns_res(buff);
        } else {
            break;
        }
    }
    cobuffer_free(buff);
}

static void io_rountine(cosched_t *sch, void *ud) {
    printf("input domain name: \n");
    
    cortenv_t *env = ud;
    cofd_t *fd = cofd_bind(env->loop->ioserivce, STDIN_FILENO, NULL);
    cort_fd_bind(env, fd);

    cobuffer_t *buff = cobuffer_new(NULL, 256, 0);
    while (true) {
        if (cort_fd_read(env, fd, buff)) {
            int sz = MAX_PACKET_LEN;
            char name[MAX_NAME_LEN] = {0};
            strncpy(name, cobuffer_buffer(buff), CO_MIN(MAX_NAME_LEN, cobuffer_pos(buff)-1));
            if (codns_pack_request(sendbuf, &sz, name)) {
                fprintf(stderr, "request error: %s\n", name);
                break;
            }
            if (udpclient) {
                coudp_send(env->loop->ioserivce, udpclient, sendbuf, sz, (struct sockaddr*)&sendaddr, addrlen);
            }
        } else {
            break;
        }
    }
    cobuffer_free(buff);
}

static void test_client(cortenv_t *env) {
    int co = cort_new(env->sch, udp_rountine, env);
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
