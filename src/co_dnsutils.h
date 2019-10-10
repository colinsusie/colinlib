#ifndef __CO_DNSUTILS__
#define __CO_DNSUTILS__
#include "co_utils.h"
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

// #define MAX_LABEL_LEN 63
// #define MAX_PACKET_LEN 512
// #define DNS_HEADER_LEN 12

// IP地址结构
typedef struct ipaddr {
    int af;
    union {
        struct in_addr a4;
	    struct in6_addr a6;
    } addr;
} ipaddr_t;

// 取DNS服务器，成功返为0
int codns_get_server(char *dnssvr, int n);

// 生成DNS请求包，成功返回0
int codns_pack_request(uint8_t *buf, int *size, const char *name);

// 解析DNS回应包，成功返回0，addrs和addrnum，外部须free(addrs)
int codns_parse_response(const uint8_t *buf, int size, ipaddr_t **addrs, int *addrnum);


#ifdef __cplusplus
}
#endif
#endif