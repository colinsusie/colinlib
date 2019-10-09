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
#define COTCP_SIZE 16384
#define COUDP_SIZE 8192
#define COFD_SIZE 1024

// socket类型
#define COIO_TCP 0
#define COIO_UDP 1
#define COIO_FD 2

// IO基类
typedef struct coio {
    int fd;                     // 文件fd
    uint8_t type;               // 类型: COIO_TCP...
} coio_t;

// 连接事件，监听socket和连接socket都会触发
//   如果是监听socket: so为新进来的socket
//   如果是连接socket: so为连接成功的socket
typedef void (*fn_tcp_connect_t)(coios_t *ss, cotcp_t* tcp);
// 收到数据的事件
typedef void (*fn_tcp_recv_t)(coios_t *ss, cotcp_t* tcp, const void *buff, int size);
// 发生错误的事件
typedef void (*fn_tcp_error_t)(coios_t *ss, cotcp_t* tcp, const char *msg);
// 关闭事件
typedef void (*fn_tcp_close_t)(coios_t *ss, cotcp_t* tcp);

// tcp状态
#define COTCP_INVALID 0
#define COTCP_RESERVE 1
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
    void *ud;                   // 用户数据
    coringbuf_t *sendbuf;       // 发送buff
    fn_tcp_connect_t fn_connect;// 各种事件
    fn_tcp_recv_t fn_recv;
    fn_tcp_error_t fn_error;
    fn_tcp_close_t fn_close;
} cotcp_t;


// udp状态：未使用
#define COUDP_INVALID 0

// udp对象
typedef struct coudp {
    int fd;                     // 文件fd
    uint8_t type;               // 类型
    uint8_t state;              // 当前状态
    void *ud;                   // 用户数据
    coringbuf_t *sendbuf;       // 发送buff
    // ...
} coudp_t;


// FD收到数据的事件
typedef void (*fn_fd_recv_t)(coios_t *ss, cofd_t* fd, const void *buff, int size);
typedef void (*fn_fd_error_t)(coios_t *ss, cofd_t* fd, const char *msg);
typedef void (*fn_fd_close_t)(coios_t *ss, cofd_t* fd);

// fd状态
#define COFD_INVALID 0
#define COFD_RESERVE 1
#define COFD_BIND 2

// 其他通用fd对象
typedef struct cofd {
    int fd;                     // 文件fd
    uint8_t type;               // 类型
    uint8_t state;              // 当前状态
    uint16_t recvsize;          // 接受buff大小
    void *recvbuf;              // 接受buff
    void *ud;                   // 用户数据
    coringbuf_t *sendbuf;       // 发送buff
    fn_fd_recv_t fn_recv;       // 事件
    fn_fd_error_t fn_error;
    fn_fd_close_t fn_close;
} cofd_t;


// socket 服务
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
// socket选项：无阻挡
void coios_nonblocking(int fd);
// 忽略信号
int coios_ignsigpipe();
// 取端点名："ip:port"
bool coios_getpeername(int fd, char *buf, int len);
// 无迟延
void cotcp_nodelay(int fd);

/////////////////////////////////////////////////////////////////////////////////////////////////
// TCP相关

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
bool cotcp_send(coios_t *ss, cotcp_t* tcp, const void *buff, int sz);
// 关闭socket, force为false表示等发送数据完了再关闭，为true则马上关闭
bool cotcp_close(coios_t *ss, cotcp_t* tcp, bool force);
// 注册错误事件
bool cotcp_on_error(coios_t *ss, cotcp_t* tcp, fn_tcp_error_t fn);
// 注册接收事件
bool cotcp_on_recv(coios_t *ss, cotcp_t* tcp, fn_tcp_recv_t fn);
// 注册关闭事件
bool cotcp_on_close(coios_t *ss, cotcp_t* tcp, fn_tcp_close_t fn);

/////////////////////////////////////////////////////////////////////////////////////////////////
// UDP相关



/////////////////////////////////////////////////////////////////////////////////////////////////
// 通用FD相关
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

#ifdef __cplusplus
}
#endif
#endif