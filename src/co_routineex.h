#ifndef __CO__ROUTINEEX__
#define __CO__ROUTINEEX__
#include "co_utils.h"
#include "co_loop.h"
#include "co_routine.h"

#ifdef __cplusplus
extern "C" {
#endif

// 协程环境
typedef struct cortenv {
    coloop_t *loop;         // 消息循环
    cosched_t *sch;         // 协程调度器
} cortenv_t;

// 初始化和释放
cortenv_t* cortenv_new(coloop_t *loop, int stsize);
void* cortenv_free(cortenv_t *env);
// 休眠ms毫秒
void cort_sleep(cortenv_t *env, uint32_t ms);

// 绑定TCP
void cort_tcp_bind(cortenv_t *env, cotcp_t *tcp);
// 从TCP读数据：阻塞直到有数据或出错或关闭
bool cort_tcp_read(cortenv_t *env, cotcp_t *tcp, cobuffer_t *buff);
// 连接服务器，阻塞直到连接上或连接不上
cotcp_t* cort_tcp_connect(cortenv_t *env, const char *ip, const char *port); 

// UDP
void cort_udp_bind(cortenv_t *env, coudp_t *udp);
// 从UDP读数据
bool cort_udp_read(cortenv_t *env, coudp_t *udp, cobuffer_t *buff);

// FD绑定
void cort_fd_bind(cortenv_t *env, cofd_t *fd);
// 从FD读数据
bool cort_fd_read(cortenv_t *env, cofd_t *fd, cobuffer_t *buff);



#ifdef __cplusplus
}
#endif
#endif