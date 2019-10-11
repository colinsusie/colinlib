# colinlib

由C语言实现的单线程高并发的网络基础库，该库实现的功能有：

- 通用的数据结构：如字典(哈希表)，向量数组，循环队列，双向链表，固定长度的内存分配器等。
- 网络编程中经常用到的buffer：可读写的buffer，环形buffer等。
- 基于时间轮算法的高效定时器。
- 独立栈的协程库，快速的执行环境切换(基于少量汇编代码)。
- 封装epoll/kqueue的高并发异步IO框架：支持tcp, udp和其他fd的异步IO。
- 整合上面的协程库以及异步IO框架，实现“同步”的读写。

该库没有使用全局变量，函数也是可重入的，所以理论上可以安全的跑在独立线程上。

# 编译以及测试

该库仅仅提供测试程序的编译：

```bash
git clone https://github.com/colinsusie/colinlib.git
cd colinlib
make
```

测试程序用于验证代码的正确性，同时也是了解代码使法的途径，下面几个测试程序重点说明：

- test_echo_server/test_echo_client echo服务器和客户端，基于事件回调的方式，用少量代码即可实现功能。
- test_echo_server2/test_echo_client2 另一个echo服务器和客户端，基于协程的同步方式，与上面相比哪种更好，由自己选择。
- test_udp_dns 异步DNS查询程序，基于事件回调的方式。
- test_udp_dns2 另一个异步DNS查询程序，基于协程的同步方式。
- test_timerservice 时间轮定时器服务，提供简单的使用接口。
- test_coroutine 协程测试程序

下面是基于回调的echo客户端的代码：

```c
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
    if (client_tcp) cotcp_send(ss, client_tcp, buf, size);
}

void run_client(coloop_t *loop) {
    // 绑定stdin到异步IO
    cofd_t *fd = cofd_bind(loop->ioserivce, STDIN_FILENO, NULL);
    cofd_on_recv(loop->ioserivce, fd, on_stdin_input);
    // 连接服务器
    cotcp_connect(loop->ioserivce, "127.0.0.1", "3458", NULL,
        on_connected,  on_connect_error);
}

int main(int argc, char const *argv[]) {
    coios_ignsigpipe();
    coloop_t *loop = coloop_new(10);
    run_client(loop);
    coloop_run(loop);
    coloop_free(loop);
    return 0;
}
```

## 最后

由于作者工作的原因，该库并未用于实际的生产环境，请谨慎选择，若只当成学习目的，那应该是相当好的。
虽然写了很多测试程序，并且用valgrind检查过内存问题，但错误在所难免，如果发现问题，欢迎提Issues。