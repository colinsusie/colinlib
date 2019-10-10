#include "co_dnsutils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#pragma pack(push)
#pragma pack(1)
typedef struct dns_header {
    uint16_t tid     ;
    uint16_t flags   ;
    uint16_t qdcount ;
    uint16_t ancount ;
    uint16_t nscount ;
    uint16_t arcount ;
} dns_header_t;
#pragma pack(pop)

#define DNS_TYPE_A 1
#define DNS_TYPE_AAAA 28
#define DNS_CLASS_IN 1

int codns_get_server(char *dnssvr, int n) {
    FILE * f = fopen("/etc/resolv.conf", "r");
    if (f == NULL) return 1;
    int ret = 1;
    char line[512];
    const char *sch = "nameserver";
    while (fgets(line, 512, f)) {
        char* ss = strstr(line, sch);
        if (!ss) continue;
        ss = strrchr(ss, 0x20);
        if (!ss) continue;
        ss++;
        char *es = strchr(ss, '\n');
        if (!es) continue;
        if (es-ss >= n) continue;
        strncpy(dnssvr, ss, es-ss);
        ret = 0;
        break;
    }
    fclose(f);
    return ret;
}

static uint16_t gtid = 1234;
inline static uint16_t gen_tid() {
    return gtid++;
}

int codns_pack_request(uint8_t *buf, int *size, const char *name) {
    int maxsize = *size;
    if (maxsize < sizeof(dns_header_t))  return 1;
    int pos = 0;

    dns_header_t header;
    memset(&header, 0, sizeof(dns_header_t));
    header.tid = htons(gen_tid());
    header.flags = htons(0x0100);           // Query, Standard Mode, Recursion Desired
    header.qdcount = htons(1);
    memcpy(buf+pos, &header, sizeof(dns_header_t));
    pos += sizeof(header);

    const char *pname = name;
    const char *ppos;
    int len;
    for (;;) {
        ppos = strchr(pname, '.');
        len = ppos ? ppos - pname : strlen(pname);
        if (maxsize < pos+1+len) return 1;
        buf[pos++] = len;
        memcpy(buf+pos, pname, len);
        pos += len;
        if (ppos)
            pname = ppos+1;
        else
            break;
    }
    if (maxsize < pos+1) return 1;
    buf[pos++] = '\0';

    uint16_t qtype = htons(DNS_TYPE_A);      // A
    if (maxsize < pos+2) return 1;
    memcpy(buf+pos, &qtype, 2);
    pos += 2;

    uint16_t qclass = htons(DNS_CLASS_IN);     // IN
    if (maxsize < pos+2) return 1;
    memcpy(buf+pos, &qclass, 2);
    pos += 2;

    *size = pos;
    return 0;
}

int codns_parse_response(const uint8_t *buf, int size, ipaddr_t **addrs, int *addrnum) {
    if (size < sizeof(dns_header_t))  return 1;
    int pos = 0;

    dns_header_t *header = (dns_header_t *)(buf + pos);
    pos += sizeof(dns_header_t);

    // rcode
    int rcode = ntohs(header->flags) & 0x000F;
    if (rcode != 0)
        return rcode;

    // query
    uint16_t qdcount = ntohs(header->qdcount);
    if (qdcount != 1) return 1;
    uint8_t len;
    // 这里要处理名字指针的情况
    while (buf[pos] != '\0') {
        if (size < pos+1) return 1;
        len = *((uint8_t*)(buf+pos));
        if ((len & 0xC0) == 0xC0) {   // name pointer
            pos++;
            break;
        } else {
            pos++;
            if (size < pos+len) return 1;
            pos += len;
        }
    }
    if (size < pos+1) return 1;
    pos++;
    if (size < pos+4) return 1;
    pos += 4;

    // answer
    uint16_t ancount = ntohs(header->ancount);
    if (ancount < 1) return 1;
    ipaddr_t *retads = CO_CALLOC(ancount, sizeof(ipaddr_t));
#define CHECK_BUF(n) if (size < pos+(n)) { free(retads); return 1;}
    
    int i;
    int num = 0;
    for (i = 0; i < ancount; ++i) {
        // 这里要处理名字指针的情况
        while (buf[pos] != '\0') {
            CHECK_BUF(1);
            len = *((uint8_t*)(buf+pos));
            if ((len & 0xC0) == 0xC0) {   // name pointer
                pos++;
                break;
            } else {
                pos++;
                CHECK_BUF(len);
                pos += len;
            }
        }
        CHECK_BUF(1);
        pos++;
        // atype
        CHECK_BUF(2);
        uint16_t atype = ntohs(*((uint16_t*)(buf+pos)));
        pos += 2;
        // skip class, ttl
        CHECK_BUF(6);
        pos += 6;
        // rdlen
        CHECK_BUF(2);
        uint16_t rdlen = ntohs(*((uint16_t*)(buf+pos)));
        pos += 2;
        // rddata
        CHECK_BUF(rdlen);
        if (atype == DNS_TYPE_A) {
            retads[num].af = AF_INET;
            memcpy(&retads[num].addr.a4, buf+pos, 4);
            num++;
        } else if (atype == DNS_TYPE_AAAA) {
            retads[num].af = AF_INET6;
            memcpy(&retads[num].addr.a6, buf+pos, 16);
            num++;
        }
        pos += rdlen;
    }

    *addrs = retads;
    *addrnum = num;
    return 0;
}