#include "co_ioservice.h"
#include "co_loop.h"
#include "co_poll.h"
#include <fcntl.h>

#define BACKLOG 128
#define INIT_READ_BUF 512
#define MAX_READ_BUF 65536

int coios_ignsigpipe() {
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	return sigaction(SIGPIPE, &sa, 0);
}

bool coios_getpeername(int fd, char *ip, int iplen, char *port, int portlen) {
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    if (getpeername(fd, (struct sockaddr*)&addr, &addrlen) == 0) {
        return coios_getaddrname((struct sockaddr*)&addr, addrlen, ip, iplen, port, portlen);
    }
    return false; 
}

bool coios_getaddrname(struct sockaddr *addr, socklen_t addrlen, char *ip, int iplen, char *port, int portlen) {
    if (getnameinfo(addr, addrlen, ip, iplen, port, portlen, NI_NUMERICHOST|NI_NUMERICSERV) == 0) {
        return true;
    }
    return false;
}

bool coios_getaddr(const char *ip, const char *port, socklen_t *addrlen, socklen_t *socklen) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *result;
    if (getaddrinfo(ip, port, &hints, &result) == 0) {
        *socklen = result->ai_addrlen;
        memcpy(addrlen, result->ai_addr, result->ai_addrlen);
        freeaddrinfo(result);
        return true;
    } else {
        freeaddrinfo(result);
        return false;
    }
}

void coios_nonblocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
	if ( -1 != flag )
		fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

void cotcp_nodelay(int fd) {
    int value = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
}

// 向前声明
void _udpbuffer_free(udpsendlist_t *list);
static void _cotcp_handle_write(coloop_t *loop, cotcp_t *tcp);
static void _cotcp_handle_read(coloop_t *loop, cotcp_t *tcp);
static void _cotcp_handle_error(coloop_t *loop, cotcp_t *tcp);
static void _cotcp_handle_connect(coloop_t *loop, cotcp_t *tcp);
static void _cotcp_handle_accept(coloop_t *loop, cotcp_t *tcp);
static void _cofd_handle_write(coloop_t *loop, cofd_t *fd);
static void _cofd_handle_read(coloop_t *loop, cofd_t *fd);
static void _cofd_handle_error(coloop_t *loop, cofd_t *fd);
static void _coudp_handle_write(coloop_t *loop, coudp_t *udp);
static void _coudp_handle_read(coloop_t *loop, coudp_t *udp);
static void _coudp_handle_error(coloop_t *loop, coudp_t *udp);



// 处理poll事件
static void _coios_process_event(coloop_t *loop, copollevent_t *ev) {
    coio_t *so = (coio_t*)ev->ud;
    if (so->type == COIO_TCP) {
        cotcp_t *tcp = (cotcp_t*)so;
        switch (tcp->state) {
        case COTCP_LISTEN:
            _cotcp_handle_accept(loop, tcp);
            break;
        case COTCP_CONNECTING:
            _cotcp_handle_connect(loop, tcp);
            break;
        case COTCP_INVALID:
            fprintf(stderr, "invalid socket -tcp\n");
            break;
        default:
            if (ev->read) {  // read
                _cotcp_handle_read(loop, tcp);
            }
            if (ev->write) {     // write
                _cotcp_handle_write(loop, tcp);
            }
            if (ev->error) {     // error
                _cotcp_handle_error(loop, tcp);
            }
        }
    } else if (so->type == COIO_UDP) {
        coudp_t *udp = (coudp_t*)so;
        if (udp->state == COFD_BIND || udp->state == COFD_NEW) {
            if (ev->read) {  // read
                _coudp_handle_read(loop, udp);
            }
            if (ev->write) {     // write
                _coudp_handle_write(loop, udp);
            }
            if (ev->error) {     // error
                _coudp_handle_error(loop, udp);
            }
        } else if (udp->state == COUDP_INVALID) {
            fprintf(stderr, "invalid socket - udp\n");
        }

    } else if (so->type == COIO_FD) {
        cofd_t *fd = (cofd_t*)so;
        if (fd->state == COFD_BIND) {
            if (ev->read) {  // read
                _cofd_handle_read(loop, fd);
            }
            if (ev->write) {     // write
                _cofd_handle_write(loop, fd);
            }
            if (ev->error) {     // error
                _cofd_handle_error(loop, fd);
            }
        } else if (fd->state == COFD_INVALID) {
            fprintf(stderr, "invalid socket - fd\n");
        }
    }
}

coios_t* coios_new(coloop_t *loop) {
    coios_t *ss = CO_MALLOC(sizeof(coios_t));
    memset(ss, 0, sizeof(*ss));
    int i;
    for (i = 0; i < COTCP_SIZE; ++i) {
        cotcp_t *tcp = &ss->tcps[i];
        tcp->type = COIO_TCP;
        tcp->ioservice = ss;
    }
    for (i = 0; i < COUDP_SIZE; ++i) {
        coudp_t *udp = &ss->udps[i];
        udp->type = COIO_UDP;
        udp->ioservice = ss;
    }
    for (i = 0; i < COFD_SIZE; ++i) {
        cofd_t *cofd = &ss->fds[i];
        cofd->type = COIO_FD; 
        cofd->ioservice = ss;
    }
    ss->loop = loop;
    coloop_processcb(loop, _coios_process_event);
    return ss;
}

void* coios_free(coios_t *ss) {
    int i;
    for (i = 0; i < COTCP_SIZE; ++i) {
        cotcp_t *tcp = &ss->tcps[i];
        if (tcp->sendbuf)
            coringbuf_free(tcp->sendbuf);
        if (tcp->recvbuf)
            CO_FREE(tcp->recvbuf);
    }
    for (i = 0; i < COUDP_SIZE; ++i) {
        coudp_t *udp = &ss->udps[i];
        if (udp->recvbuf)
            CO_FREE(udp->recvbuf);
        _udpbuffer_free(&udp->sendlist);
    }
    for (i = 0; i < COFD_SIZE; ++i) {
        cofd_t *fd = &ss->fds[i];
        if (fd->sendbuf)
            coringbuf_free(fd->sendbuf);
        if (fd->recvbuf)
            CO_FREE(fd->recvbuf);
    }
    CO_FREE(ss);
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
static cotcp_t* _cotcp_alloc(coios_t *ss) {
    int i, index;
    for (i = 0; i < COTCP_SIZE; ++i) {
        index = (ss->tcpid++) % COTCP_SIZE;
        cotcp_t *tcp = &ss->tcps[index];
        if (tcp->state == COTCP_INVALID) {
            tcp->state = COTCP_NEW;
            return tcp;
        }
    }
    return NULL;
}

static void _cotcp_free(coios_t *ss, cotcp_t *tcp) {
    if (tcp->state == COTCP_INVALID)
        return;
    if (tcp->sendbuf)
        coringbuf_free(tcp->sendbuf);
    if (tcp->recvbuf)
        CO_FREE(tcp->recvbuf);
    memset(tcp, 0, sizeof(*tcp));
    tcp->type = COIO_TCP;
    tcp->ioservice = ss;
}

static bool _cotcp_has_senddata(cotcp_t *tcp) {
    return tcp->sendbuf && coringbuf_readable_size(tcp->sendbuf);
}

static void _cotcp_close(coios_t *ss, cotcp_t *tcp) {
    if (tcp->state == COTCP_INVALID)
        return;
    copoll_del(ss->loop->poll, tcp->fd);
    if (close(tcp->fd) < 0)
        perror("_cotcp_close - close");
    _cotcp_free(ss, tcp);
}

static void _cotcp_handle_error(coloop_t *loop, cotcp_t *tcp) {
    if (tcp->state == COTCP_INVALID)
        return;
    const char * err = NULL;
    int eno = errno;
    if (eno)
        err = strerror(eno);
    else {
        int error;
        socklen_t len = sizeof(error);  
        int code = getsockopt(tcp->fd, SOL_SOCKET, SO_ERROR, &error, &len);  
        if (code)
            err = strerror(error);
        else
            err = "Unknown error";
    }
    if (tcp->fn_error) {
        tcp->fn_error(loop->ioserivce, tcp, err);
    }
    _cotcp_close(loop->ioserivce, tcp);
}

static void _cotcp_handle_accept(coloop_t *loop, cotcp_t *tcp) {
    struct sockaddr_storage claddr;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    for (;;) {
        int fd = accept(tcp->fd, (struct sockaddr*)&claddr, &addrlen);
        if (fd < 0) {
            int no = errno;
            if (no == EINTR)
                continue;
            perror("_cotcp_handle_accept - accept");
            return;
        }
        cotcp_t *ctcp = _cotcp_alloc(loop->ioserivce);
        if (!ctcp) {
            fprintf(stderr, "too many tcp\n");
            close(fd);
            return;
        }
        coios_nonblocking(fd);
        ctcp->fd = fd;
        ctcp->ud = tcp->ud;
        ctcp->state = COTCP_CONNECTED;
        copoll_add(loop->poll, fd, ctcp);
        if (tcp->fn_connect)
            tcp->fn_connect(loop->ioserivce, ctcp);
        return;
    }
}

static void _cotcp_handle_connect(coloop_t *loop, cotcp_t *tcp) {
    int error;
	socklen_t len = sizeof(error);  
	int code = getsockopt(tcp->fd, SOL_SOCKET, SO_ERROR, &error, &len);  
	if (code < 0 || error) {  
        int eno = code >= 0 ? error : errno;
        if (tcp->fn_error)
            tcp->fn_error(loop->ioserivce, tcp, strerror(eno));
        _cotcp_close(loop->ioserivce, tcp);
    } else {
        tcp->state = COTCP_CONNECTED;
        // 连接成功后，会将错误事件重置，需要再手动设置一次
        tcp->fn_error = NULL;
        if (tcp->fn_connect)
            tcp->fn_connect(loop->ioserivce, tcp);
    }
}

static void _cotcp_handle_read(coloop_t *loop, cotcp_t *tcp) {
    if (tcp->state == COTCP_INVALID)
        return;
    if (!tcp->recvbuf) {
        tcp->recvsize = INIT_READ_BUF;
        tcp->recvbuf = CO_MALLOC(tcp->recvsize);
    }
    while (1) {
        ssize_t n = read(tcp->fd, tcp->recvbuf, tcp->recvsize);
        if (n < 0) {
            int no = errno;
            if (no == EINTR)
                continue;
            else if (no == EAGAIN || no == EWOULDBLOCK)   // 缓冲满了，下次再来
                return;
            else {      // 其他错误
                _cotcp_handle_error(loop, tcp);
                return;
            }
        } else if (n == 0) {        // socket关闭
            if (tcp->fn_close)
                tcp->fn_close(loop->ioserivce, tcp);
            _cotcp_close(loop->ioserivce, tcp);
            return;
        } else {
            // 连接状态下才触发事件
            if (tcp->state == COTCP_CONNECTED) {
                if (tcp->fn_recv)
                    tcp->fn_recv(loop->ioserivce, tcp, tcp->recvbuf, n);
                if (tcp->state != COTCP_INVALID && n == tcp->recvsize) {
                    tcp->recvsize = CO_MIN(MAX_READ_BUF, tcp->recvsize*2);
                    tcp->recvbuf = CO_REALLOC(tcp->recvbuf, tcp->recvsize);
                }
            }
            return;
        }
    }
}

static void _cotcp_check_closing(coloop_t *loop, cotcp_t *tcp) {
    if (tcp->state == COTCP_CLOSING) {      // 关闭socket
        _cotcp_close(loop->ioserivce, tcp);
    } else {
        copoll_request_write(loop->poll, tcp->fd, tcp, false);
    }
}

static void _cotcp_handle_write(coloop_t *loop, cotcp_t *tcp) {
    if (tcp->state == COTCP_INVALID)
        return;
    int maxsz = 2048;
    int size;
    while (_cotcp_has_senddata(tcp)) {
        size = CO_MIN(maxsz, coringbuf_readonce_size(tcp->sendbuf));
        if (!size)
            break;
        void *buff = coringbuf_head(tcp->sendbuf);
        int nwrite = write(tcp->fd, buff, size);
        if (nwrite < 0) {
            int no = errno;
            if (no == EINTR)        // 信号中断，继续
                continue;
            else if (no == EAGAIN || no == EWOULDBLOCK)   // 缓冲满了，下次再来
                return;
            else {  // 其他错误 
                _cotcp_handle_error(loop, tcp);
                return;
            }
        }
        coringbuf_consume_size(tcp->sendbuf, nwrite);
        if (nwrite != size)
            return;
    }

    // 已经写完，关闭监听
    _cotcp_check_closing(loop, tcp);
}

cotcp_t* cotcp_listen(coios_t *ss, const char *ip, const char *port, void *ud, fn_tcp_connect_t fn_connect) {
    cotcp_t *tcp = _cotcp_alloc(ss);
    if (!tcp) return NULL;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 

    struct addrinfo *result;
    int s = getaddrinfo(ip, port, &hints, &result);
    if (s != 0) {
        _cotcp_free(ss, tcp);
        errno = ENOSYS;
        return NULL;
    }

    int optval = 1;
    int fd = 0;
    struct addrinfo *rp;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd == -1)
            continue;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
            perror("cotcp_listen - setsockopt");
            close(fd);
            freeaddrinfo(result);
            _cotcp_free(ss, tcp);
            return NULL;
        }
        if (bind(fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;
        close(fd);
    }

    if (rp == NULL) {
        freeaddrinfo(result);
        _cotcp_free(ss, tcp);
        return NULL;
    }

    if (listen(fd, BACKLOG) == -1) {
        perror("cotcp_listen - listen");
        close(fd);
        freeaddrinfo(result);
        _cotcp_free(ss, tcp);
        return NULL;
    }

    freeaddrinfo(result);
    tcp->fd = fd;
    tcp->ud = ud;
    tcp->fn_connect = fn_connect;
    tcp->state = COTCP_LISTEN;
    copoll_add(ss->loop->poll, fd, tcp);
    return tcp;
}

bool cotcp_connect(coios_t *ss, const char *ip, const char *port, void *ud, fn_tcp_connect_t fn_connect,
    fn_tcp_error_t fn_error) {
    cotcp_t *tcp = _cotcp_alloc(ss);
    if (!tcp) return false;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *result;
    int s = getaddrinfo(ip, port, &hints, &result);
    if (s != 0) {
        _cotcp_free(ss, tcp);
        errno = ENOSYS;
        return false;
    }

    int fd = 0;
    struct addrinfo *rp;
    int ret;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd == -1)
            continue;
        coios_nonblocking(fd);
        ret = connect(fd, rp->ai_addr, rp->ai_addrlen);
        if (ret != 0 && errno != EINPROGRESS) {
            close(fd);
            continue;
        }
        break;
    }

    if (rp == NULL) {
        perror("cotcp_connect - connect");
        freeaddrinfo(result);
        _cotcp_free(ss, tcp);
        return false;
    }

    freeaddrinfo(result);
    tcp->fd = fd;
    tcp->ud = ud;
    tcp->fn_connect = fn_connect;
    tcp->fn_error = fn_error;
    copoll_add(ss->loop->poll, fd, tcp);
    if (ret == 0) {
        tcp->state = COTCP_CONNECTED;
        if (tcp->fn_connect)
            tcp->fn_connect(ss, tcp);
    } else {
        tcp->state = COTCP_CONNECTING;      // 正在连接，等待连接事件到来
        copoll_request_write(ss->loop->poll, fd, tcp, true);
    }
    return true;
}

bool cotcp_send(coios_t *ss, cotcp_t *tcp, const void *buff, int size) {
    if (tcp->state != COTCP_CONNECTED || !size)
        return false;
    if (!tcp->sendbuf)
        tcp->sendbuf = coringbuf_new(size*2);
    coringbuf_write(tcp->sendbuf, buff, size);
    copoll_request_write(ss->loop->poll, tcp->fd, tcp, true);
    // 先尝试直接发送，发送不成功再进入监听队列
    _cotcp_handle_write(ss->loop, tcp);
    return true;
}

bool cotcp_close(coios_t *ss, cotcp_t *tcp, bool force) {
    if (tcp->state > COTCP_NEW && tcp->state < COTCP_CLOSING) {
        // 如果有数据，尝试发送一次
        if (_cotcp_has_senddata(tcp))
            _cotcp_handle_write(ss->loop, tcp);
        if (force || !_cotcp_has_senddata(tcp)) {
            if (tcp->fn_close)
                tcp->fn_close(ss, tcp);
            _cotcp_close(ss, tcp);
        } else {
            tcp->state = COTCP_CLOSING;
        }
        return true;
    }
    return false;
}

bool cotcp_on_error(coios_t *ss, cotcp_t *tcp, fn_tcp_error_t fn) {
    if (tcp->state > COTCP_NEW && tcp->state < COTCP_CLOSING) {
        tcp->fn_error = fn;
        return true;
    }
    return false;
}

bool cotcp_on_recv(coios_t *ss, cotcp_t *tcp, fn_tcp_recv_t fn) {
    if (tcp->state > COTCP_LISTEN && tcp->state < COTCP_CLOSING) {
        tcp->fn_recv = fn;
        return true;
    }
    return false;
}

bool cotcp_on_close(coios_t *ss, cotcp_t* tcp, fn_tcp_close_t fn) {
    if (tcp->state > COTCP_NEW && tcp->state < COTCP_CLOSING) {
        tcp->fn_close = fn;
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// fd

static cofd_t* _cofd_alloc(coios_t *ss) {
    int i, index;
    for (i = 0; i < COFD_SIZE; ++i) {
        index = (ss->fdid++) % COFD_SIZE;
        cofd_t *fd = &ss->fds[index];
        if (fd->state == COFD_INVALID) {
            fd->state = COFD_NEW;
            return fd;
        }
    }
    return NULL;
}

static void _cofd_free(coios_t *ss, cofd_t *fd) {
    if (fd->state == COFD_INVALID)
        return;
    if (fd->sendbuf)
        coringbuf_free(fd->sendbuf);
    if (fd->recvbuf)
        CO_FREE(fd->recvbuf);
    memset(fd, 0, sizeof(*fd));
    fd->type = COIO_FD;
    fd->ioservice = ss;
}

static bool _cofd_has_senddata(cofd_t *fd) {
    return fd->sendbuf && coringbuf_readable_size(fd->sendbuf);
}

cofd_t* cofd_bind(coios_t *ss, int fd, void *ud) {
    cofd_t *cofd = _cofd_alloc(ss);
    if (!cofd) return NULL;

    coios_nonblocking(fd);
    cofd->fd = fd;
    cofd->ud = ud;
    cofd->state = COFD_BIND;
    copoll_add(ss->loop->poll, fd, cofd);
    return cofd;
}

// 反绑FD
bool cofd_unbind(coios_t *ss, cofd_t *fd) {
    // 马上解除
    if (fd->state == COFD_BIND) {
        copoll_del(ss->loop->poll, fd->fd);
        _cofd_free(ss, fd);
        return true;
    }
    return false;
}

// 发送数据
bool cofd_send(coios_t *ss, cofd_t *fd, void *buf, int size) {
    if (fd->state != COFD_BIND || !size)
        return false;
    if (!fd->sendbuf)
        fd->sendbuf = coringbuf_new(size*2);
    coringbuf_write(fd->sendbuf, buf, size);
    copoll_request_write(ss->loop->poll, fd->fd, fd->ud, true);
    // 先尝试直接发送，发送不成功再进入监听队列
    _cofd_handle_write(ss->loop, fd);
    return true;
}

// 注册错误事件
bool cofd_on_error(coios_t *ss, cofd_t* fd, fn_fd_error_t fn) {
    if (fd->state == COFD_BIND) {
        fd->fn_error = fn;
        return true;
    }
    return false;
}

// 注册接收事件
bool cofd_on_recv(coios_t *ss, cofd_t* fd, fn_fd_recv_t fn) {
    if (fd->state == COFD_BIND) {
        fd->fn_recv = fn;
        return true;
    }
    return false;
}

// 注册关闭事件
bool cofd_on_close(coios_t *ss, cofd_t* fd, fn_fd_close_t fn) {
    if (fd->state == COFD_BIND) {
        fd->fn_close = fn;
        return true;
    }
    return false;
}

static void _cofd_handle_read(coloop_t *loop, cofd_t *fd) {
    if (fd->state == COFD_INVALID)
        return;
    if (!fd->recvbuf) {
        fd->recvsize = INIT_READ_BUF;
        fd->recvbuf = CO_MALLOC(fd->recvsize);
    }
    while (1) {
        ssize_t n = read(fd->fd, fd->recvbuf, fd->recvsize);
        if (n < 0) {
            int no = errno;
            if (no == EINTR)
                continue;
            else if (no == EAGAIN || no == EWOULDBLOCK)   // 缓冲满了，下次再来
                return;
            else {      // 其他错误
                _cofd_handle_error(loop, fd);
                return;
            }
        } else if (n == 0) {        // socket关闭
            if (fd->fn_close)
                fd->fn_close(loop->ioserivce, fd);
            cofd_unbind(loop->ioserivce, fd);
            return;
        } else {
            if (fd->fn_recv)
                fd->fn_recv(loop->ioserivce, fd, fd->recvbuf, n);
            if (fd->state != COFD_INVALID &&n == fd->recvsize) {
                fd->recvsize = CO_MIN(MAX_READ_BUF, fd->recvsize*2);
                fd->recvbuf = CO_REALLOC(fd->recvbuf, fd->recvsize);
            }
            return;
        }
    }
}

static void _cofd_handle_write(coloop_t *loop, cofd_t *fd) {
    if (fd->state == COFD_INVALID)
        return;
    int maxsz = 2048;
    int size;
    while (_cofd_has_senddata(fd)) {
        size = CO_MIN(maxsz, coringbuf_readonce_size(fd->sendbuf));
        if (!size)
            break;
        void *buff = coringbuf_head(fd->sendbuf);
        int nwrite = write(fd->fd, buff, size);
        if (nwrite < 0) {
            int no = errno;
            if (no == EINTR)        // 信号中断，继续
                continue;
            else if (no == EAGAIN || no == EWOULDBLOCK)   // 缓冲满了，下次再来
                return;
            else {  // 其他错误 
                _cofd_handle_error(loop, fd);
                return;
            }
        }
        coringbuf_consume_size(fd->sendbuf, nwrite);
        if (nwrite != size)
            return;
    }

    // 已经写完，关闭监听
    copoll_request_write(loop->poll, fd->fd, fd->ud, false);        
}

static void _cofd_handle_error(coloop_t *loop, cofd_t *fd) {
    if (fd->state == COFD_INVALID)
        return;
    const char * err = NULL;
    int eno = errno;
    if (eno)
        err = strerror(eno);
    else {
        int error;
        socklen_t len = sizeof(error);  
        int code = getsockopt(fd->fd, SOL_SOCKET, SO_ERROR, &error, &len);  
        if (code)
            err = strerror(error);
        else
            err = "Unknown error";
    }
    if (fd->fn_error) {
        fd->fn_error(loop->ioserivce, fd, err);
    }
    cofd_unbind(loop->ioserivce, fd);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// udp
static coudp_t* _coudp_alloc(coios_t *ss) {
    int i, index;
    for (i = 0; i < COUDP_SIZE; ++i) {
        index = (ss->udpid++) % COUDP_SIZE;
        coudp_t *udp = &ss->udps[index];
        if (udp->state == COUDP_INVALID) {
            udp->state = COUDP_NEW;
            return udp;
        }
    }
    return NULL;
}

static void _coudp_free(coios_t *ss, coudp_t *udp) {
    if (udp->state == COUDP_INVALID)
        return;
    if (udp->recvbuf)
        CO_FREE(udp->recvbuf);
    _udpbuffer_free(&udp->sendlist);
    memset(udp, 0, sizeof(*udp));
    udp->type = COIO_UDP;
    udp->ioservice = ss;
}

void _udpbuffer_add(udpsendlist_t *list, const void *buff, int size, struct sockaddr *addr, socklen_t addrlen) {
    udpbuffer_t *wb = CO_MALLOC(sizeof(*wb));
    wb->buffer = CO_MALLOC(size);
    memcpy(wb->buffer, buff, size);
    wb->ptr = wb->buffer;
    wb->size = size;
    wb->addr = CO_MALLOC(addrlen);
    wb->addrlen = addrlen;
    memcpy(wb->addr, addr, addrlen);
    wb->next = NULL;
    if (!list->head) {
        list->head = list->tail = wb;
    } else {
        list->tail->next = wb;
        list->tail = wb;
    }
}

void _udpbuffer_free(udpsendlist_t *list) {
    udpbuffer_t *wb = list->head;
    while (wb) {
        udpbuffer_t *tmp = wb;
        wb = wb->next;
        CO_FREE(tmp->addr);
        CO_FREE(tmp->buffer);
        CO_FREE(tmp);
    }
    list->head = NULL;
    list->tail = NULL;
}

static void _coudp_close(coios_t *ss, coudp_t *udp) {
    if (udp->state == COUDP_INVALID)
        return;
    copoll_del(ss->loop->poll, udp->fd);
    if (close(udp->fd) < 0)
        perror("_coudp_close - close");
    _coudp_free(ss, udp);
}

// 新建UDP对象并绑定到一个地址上
coudp_t* coudp_new(coios_t *ss, const char *ip, const char *port, void *ud, bool bindaddr, 
    struct sockaddr *addr, socklen_t *addrlen) {
    coudp_t *udp = _coudp_alloc(ss);
    if (!udp) return NULL;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; 

    struct addrinfo *result;
    int s = getaddrinfo(ip, port, &hints, &result);
    if (s != 0) {
        errno = ENOSYS;
        _coudp_free(ss, udp);
        return NULL;
    }

    int optval = 1;
    int fd = 0;
    struct addrinfo *rp;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd == -1)
            continue;
        if (bindaddr && setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
            perror("coudp_new - setsockopt");
            close(fd);
            freeaddrinfo(result);
            _coudp_free(ss, udp);
            return NULL;
        }
        if (!bindaddr || bind(fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;
        close(fd);
    }

    if (rp == NULL) {
        perror("coudp_new - bind");
        freeaddrinfo(result);
        _coudp_free(ss, udp);
        return NULL;
    }

    if (addr && addrlen) {
        *addrlen = rp->ai_addrlen;
        memcpy(addr, rp->ai_addr, rp->ai_addrlen);
    }
    freeaddrinfo(result);
    coios_nonblocking(fd);
    udp->fd = fd;
    udp->ud = ud;
    udp->state = COUDP_NEW;
    copoll_add(ss->loop->poll, fd, udp);
    return udp;
}

// 发送数据到一个地址
bool coudp_send(coios_t *ss, coudp_t *udp, const void *buff, int size, struct sockaddr *addr, socklen_t addrlen) {
    if (udp->state != COUDP_NEW || !size)
        return false;
    // 加入缓存
    _udpbuffer_add(&udp->sendlist, buff, size, addr, addrlen);
    copoll_request_write(ss->loop->poll, udp->fd, udp, true);
    // 先尝试直接发送，发送不成功再进入监听队列
    _coudp_handle_write(ss->loop, udp);
    return true;
}

static bool _coudp_has_senddata(coudp_t *udp) {
    return udp->sendlist.head != NULL;
}

// 关闭UDP
bool coudp_close(coios_t *ss, coudp_t *udp, bool force) {
    if (udp->state >= COUDP_NEW && udp->state < COUDP_CLOSING) {
        // 如果有数据，尝试发送一次
        if (_coudp_has_senddata(udp))
            _coudp_handle_write(ss->loop, udp);
        if (force || !_coudp_has_senddata(udp)) {
            if (udp->fn_close)
                udp->fn_close(ss, udp);
            _coudp_close(ss, udp);
        } else {
            udp->state = COUDP_CLOSING;
        }
        return true;
    }
    return false;
}

// 注册错误事件
bool coudp_on_error(coios_t *ss, coudp_t* udp, fn_udp_error_t fn) {
    if (udp->state == COUDP_NEW) {
        udp->fn_error = fn;
        return true;
    }
    return false;
}

// 注册接收事件
bool coudp_on_recv(coios_t *ss, coudp_t* udp, fn_udp_recv_t fn) {
    if (udp->state == COUDP_NEW) {
        udp->fn_recv = fn;
        return true;
    }
    return false;
}

// 注册关闭事件
bool coudp_on_close(coios_t *ss, coudp_t* udp, fn_udp_close_t fn) {
    if (udp->state == COUDP_NEW) {
        udp->fn_close = fn;
        return true;
    }
    return false;
}

static void _coudp_check_closing(coloop_t *loop, coudp_t *udp) {
    if (udp->state == COUDP_CLOSING) {      // 关闭socket
        _coudp_close(loop->ioserivce, udp);
    } else {
        copoll_request_write(loop->poll, udp->fd, udp, false);
    }
}

static void _coudp_handle_write(coloop_t *loop, coudp_t *udp) {
    if (udp->state == COUDP_INVALID)
        return;
    udpsendlist_t *list = &udp->sendlist;
    while (list->head) {
        udpbuffer_t *wb = list->head;
        for (;;) {
            ssize_t sz = sendto(udp->fd, wb->ptr, wb->size, 0, wb->addr, wb->addrlen);
            if (sz < 0) {
                int no = errno;
                if (no == EINTR)        // 信号中断，继续
                    continue;
                else if (no == EAGAIN || no == EWOULDBLOCK)   // 内核缓冲满了，下次再来
                    return;
                else {      // 其他错误 
                    _coudp_handle_error(loop, udp);
                    return;
                }
            }
            if (sz != wb->size) {       // 未完全发送出去，下次再来
                wb->ptr += sz;
                wb->size -= sz;
                return;
            }
            break;
        }
        list->head = wb->next;
        CO_FREE(wb->buffer);
        CO_FREE(wb->addr);
        CO_FREE(wb);
    }
    list->tail = NULL;
    // 到这里写全部完成，关闭写事件
    _coudp_check_closing(loop, udp);
}

static void _coudp_handle_read(coloop_t *loop, coudp_t *udp) {
    if (udp->state == COUDP_INVALID)
        return;
    if (!udp->recvbuf) {
        udp->recvsize = INIT_READ_BUF;
        udp->recvbuf = CO_MALLOC(udp->recvsize);
    }
    while (1) {
        struct sockaddr_storage claddr;
        socklen_t addrlen = sizeof(struct sockaddr_storage);
        ssize_t n = recvfrom(udp->fd, udp->recvbuf, udp->recvsize, 0, (struct sockaddr*) &claddr, &addrlen);
        if (n < 0) {
            int no = errno;
            if (no == EINTR)
                continue;
            else if (no == EAGAIN || no == EWOULDBLOCK)   // 缓冲满了，下次再来
                return;
            else {  // 其他错误
                _coudp_handle_error(loop, udp);
                return;
            }
        } else if (n == 0) {        // socket关闭
            if (udp->fn_close)
                udp->fn_close(loop->ioserivce, udp);
            _coudp_close(loop->ioserivce, udp);
            return;
        } else {
            if (udp->fn_recv)
                udp->fn_recv(loop->ioserivce, udp, udp->recvbuf, n, (struct sockaddr*) &claddr, addrlen);
            if (udp->state != COUDP_INVALID &&n == udp->recvsize) {
                udp->recvsize = CO_MIN(MAX_READ_BUF, udp->recvsize*2);
                udp->recvbuf = CO_REALLOC(udp->recvbuf, udp->recvsize);
            }
            return;
        }
    }
}

static void _coudp_handle_error(coloop_t *loop, coudp_t *udp) {
    if (udp->state == COUDP_INVALID)
        return;
    const char * err = NULL;
    int eno = errno;
    if (eno)
        err = strerror(eno);
    else {
        int error;
        socklen_t len = sizeof(error);  
        int code = getsockopt(udp->fd, SOL_SOCKET, SO_ERROR, &error, &len);  
        if (code)
            err = strerror(error);
        else
            err = "Unknown error";
    }
    if (udp->fn_error) {
        udp->fn_error(loop->ioserivce, udp, err);
    }
    _coudp_close(loop->ioserivce, udp);
}