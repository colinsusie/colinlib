/**
 * io服务框架
 *              by colin
 */
#ifndef __CO_IOSERVICE__
#define __CO_IOSERVICE__
#include "co_utils.h"
#include "co_buffer.h"
#include "co_vec.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

// 声明
struct coloop;
typedef struct coloop coloop_t;
struct cotcp;
typedef struct cotcp cotcp_t;
struct coudp;
typedef struct coudp coudp_t;
struct cofd;
typedef struct cofd cofd_t;
struct coios;
typedef struct coios coios_t;

// 预留大小
#define COTCP_SIZE 8192
#define COUDP_SIZE 4096
#define COFD_SIZE 1024

// socket类型
#define COIO_TCP 0
#define COIO_UDP 1
#define COIO_FD 2

// IO基类
typedef struct coio {
    int fd;                     // 文件fd
    uint8_t type;               // 类型: COIO_TCP...
    uint8_t state;              // 当前状态
    uint16_t recvsize;          // 接受buff大小
    void *recvbuf;              // 接受buff
    coios_t *ioservice;         // IO service
    void *ud;                   // 用户数据
} coio_t;

/////////////////////////////////////////////////////////////////////////////////////////////////
// tcp
// 连接事件，监听socket和连接socket都会触发
//   如果是监听socket: tcp为新进来的socket
//   如果是连接socket: tcp为连接成功的socket
typedef void (*fn_tcp_connect_t)(coios_t *ss, cotcp_t* tcp);
// 收到数据的事件
typedef void (*fn_tcp_recv_t)(coios_t *ss, cotcp_t* tcp, const void *buff, int size);
// 发生错误的事件
typedef void (*fn_tcp_error_t)(coios_t *ss, cotcp_t* tcp, const char *msg);
// 关闭事件
typedef void (*fn_tcp_close_t)(coios_t *ss, cotcp_t* tcp);

// tcp状态
#define COTCP_INVALID 0
#define COTCP_NEW 1
#define COTCP_LISTEN 2
#define COTCP_CONNECTING 3
#define COTCP_CONNECTED 4
#define COTCP_CLOSING 5

// tcp对象
typedef struct cotcp {
    int fd;                     // 文件fd
    uint8_t type;               // 类型
    uint8_t state;              // 当前状态
    uint16_t recvsize;          // 接受buff大小
    void *recvbuf;              // 接受buff
    coios_t *ioservice;         // IO service
    void *ud;                   // 用户数据
    coringbuf_t *sendbuf;       // 发送buff
    fn_tcp_connect_t fn_connect;// 各种事件
    fn_tcp_recv_t fn_recv;
    fn_tcp_error_t fn_error;
    fn_tcp_close_t fn_close;
} cotcp_t;

// 创建TCP监听socket，返回cotcp_t*
cotcp_t* cotcp_listen(coios_t *ss, 
    const char *ip, const char *port, void *ud,   // ip, port, ud
    fn_tcp_connect_t fn_connect                   // 有新连接进来会回调
);
// 连接远程服务器
bool cotcp_connect(coios_t *ss, 
    const char *ip, const char *port, void *ud,
    fn_tcp_connect_t fn_connect,                   // 连接成功会回调
    fn_tcp_error_t fn_error                        // 连接失败会回调
);
// 发送数据
bool cotcp_send(coios_t *ss, cotcp_t* tcp, const void *buff, int size);
// 关闭socket, force为false表示等发送数据完了再关闭，为true则马上关闭
bool cotcp_close(coios_t *ss, cotcp_t* tcp, bool force);
// 注册错误事件
bool cotcp_on_error(coios_t *ss, cotcp_t* tcp, fn_tcp_error_t fn);
// 注册接收事件
bool cotcp_on_recv(coios_t *ss, cotcp_t* tcp, fn_tcp_recv_t fn);
// 注册关闭事件
bool cotcp_on_close(coios_t *ss, cotcp_t* tcp, fn_tcp_close_t fn);


/////////////////////////////////////////////////////////////////////////////////////////////////
// udp
// 收到数据的事件: 
typedef void (*fn_udp_recv_t)(coios_t *ss, coudp_t* udp, const void *buff, int size, struct sockaddr *addr, socklen_t addrlen);
// 发生错误的事件
typedef void (*fn_udp_error_t)(coios_t *ss, coudp_t* udp, const char *msg);
// 关闭事件
typedef void (*fn_udp_close_t)(coios_t *ss, coudp_t* udp);

// UDP缓存结点
typedef struct udpbuffer {
    struct udpbuffer *next;   // 下一个缓存
    struct sockaddr *addr;    // 地址信息
    socklen_t addrlen;        // 地址长度
    void *buffer;             // 缓存
    char *ptr;                // 当前未发送的缓存，buffer != ptr表示只发送了一部分
    int size;                 // 当前未发送的缓存大小
} udpbuffer_t;

// UDP缓存列表
typedef struct udpsendlist {
    struct udpbuffer *head;
    struct udpbuffer *tail;
} udpsendlist_t;

// udp状态
#define COUDP_INVALID 0
#define COUDP_NEW 1
#define COUDP_CLOSING 2

// udp对象
typedef struct coudp {
    int fd;                     // 文件fd
    uint8_t type;               // 类型
    uint8_t state;              // 当前状态
    uint16_t recvsize;          // 接受buff大小
    void *recvbuf;              // 接受buff
    coios_t *ioservice;         // IO service
    void *ud;                   // 用户数据
    udpsendlist_t sendlist;     // 发送列表
    fn_udp_recv_t fn_recv;
    fn_udp_error_t fn_error;
    fn_udp_close_t fn_close;
} coudp_t;

// 新建UDP对象， bindaddr指定是否绑定到一个地址上
coudp_t* coudp_new(coios_t *ss, const char *ip, const char *port, void *ud, 
    bool bindaddr, struct sockaddr *addr, socklen_t *addrlen);
// 发送数据到一个地址
bool coudp_send(coios_t *ss, coudp_t *udp, const void *buff, int size, struct sockaddr *addr, socklen_t addrlen);
// 关闭UDP
bool coudp_close(coios_t *ss, coudp_t *udp, bool force);
// 注册错误事件
bool coudp_on_error(coios_t *ss, coudp_t* udp, fn_udp_error_t fn);
// 注册接收事件
bool coudp_on_recv(coios_t *ss, coudp_t* udp, fn_udp_recv_t fn);
// 注册关闭事件
bool coudp_on_close(coios_t *ss, coudp_t* udp, fn_udp_close_t fn);


/////////////////////////////////////////////////////////////////////////////////////////////////
// fd：用于绑定已有的FD对象
// FD收到数据的事件
typedef void (*fn_fd_recv_t)(coios_t *ss, cofd_t* fd, const void *buff, int size);
typedef void (*fn_fd_error_t)(coios_t *ss, cofd_t* fd, const char *msg);
typedef void (*fn_fd_close_t)(coios_t *ss, cofd_t* fd);

// fd状态
#define COFD_INVALID 0
#define COFD_NEW 1
#define COFD_BIND 2

// 其他通用fd对象
typedef struct cofd {
    int fd;                     // 文件fd
    uint8_t type;               // 类型
    uint8_t state;              // 当前状态
    uint16_t recvsize;          // 接受buff大小
    void *recvbuf;              // 接受buff
    coios_t *ioservice;         // IO service
    void *ud;                   // 用户数据
    coringbuf_t *sendbuf;       // 发送buff
    fn_fd_recv_t fn_recv;       // 事件
    fn_fd_error_t fn_error;
    fn_fd_close_t fn_close;
} cofd_t;

// 绑定FD
cofd_t* cofd_bind(coios_t *ss, int fd, void *ud);
// 反绑FD
bool cofd_unbind(coios_t *ss, cofd_t *fd);
// 发送数据
bool cofd_send(coios_t *ss, cofd_t *fd, void *buf, int size);
// 注册错误事件
bool cofd_on_error(coios_t *ss, cofd_t* fd, fn_fd_error_t fn);
// 注册接收事件
bool cofd_on_recv(coios_t *ss, cofd_t* fd, fn_fd_recv_t fn);
// 注册关闭事件
bool cofd_on_close(coios_t *ss, cofd_t* fd, fn_fd_close_t fn);


/////////////////////////////////////////////////////////////////////////////////////////////////
// 异步IO服务
typedef struct coios {
    coloop_t *loop;                 // 事件循环
    cotcp_t tcps[COTCP_SIZE];       // 预留的各种对象
    coudp_t udps[COUDP_SIZE];
    cofd_t fds[COFD_SIZE];
    uint32_t tcpid;                 // 当前分配的索引
    uint32_t udpid;
    uint32_t fdid;
} coios_t;

// socket服务初始化和释放
coios_t* coios_new(coloop_t *loop);
void* coios_free(coios_t *ss);
// socket选项：无阻塞
void coios_nonblocking(int fd);
// 忽略信号
int coios_ignsigpipe();
// 通过fd取地址字符串
bool coios_getpeername(int fd, char *ip, int iplen, char *port, int portlen);
// 通过addr取地址字符串，表现形式为：ip:port或[ipv6]:port
bool coios_getaddrname(struct sockaddr *addr, socklen_t addrlen, char *ip, int iplen, char *port, int portlen);
// 通过ip和port获得地址结构
bool coios_getaddr(const char *ip, const char *port, socklen_t *addrlen, socklen_t *socklen);
// TCP无迟延
void cotcp_nodelay(int fd);

#ifdef __cplusplus
}
#endif
#endif