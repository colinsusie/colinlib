/**
 * 各种类型的buffer
 *                  by colin
 */
#ifndef __CO_BUFFER__
#define __CO_BUFFER__
#include "co_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

// 字节序
typedef enum cb_endian {
    CB_EN_NATIVE,       // 主机序
    CB_EN_LITTLE,       // 小端
    CB_EN_BIG,          // 大端
} cb_endian_t;

// 打印BUFF
void coprintfbuffer(const void *buffer, int size, int blocksize);

///////////////////////////////////////////////////////////////////////////////////
// 读buffer：通常用于解析格式化数据

// 只读buffer
typedef struct corb {
    void *buffer;
    int size;
    int pos;
    cb_endian_t endian;
} corb_t;

// 挂载buffer
void corb_attach(corb_t *rb, void *buffer, int size, cb_endian_t endian);
// 定位: abs指定是否绝对定位 false表示相对于当前pos，绝对定位支持-1,-2表示最后，返回实际的定位
int corb_seek(corb_t *rb, bool abs, int pos);
// buff大小位置等
static inline void* corb_buffer(corb_t *rb) { return rb->buffer; }
static inline int corb_size(corb_t *rb) { return rb->size; }
static inline int corb_pos(corb_t *rb) { return rb->pos; }
static inline int corb_remainder(corb_t *rb) { return rb->size - rb->pos; }
// 读各种类型
int8_t corb_read_int8(corb_t *rb, bool *succ);
uint8_t corb_read_uint8(corb_t *rb, bool *succ);
int16_t corb_read_int16(corb_t *rb, bool *succ);
uint16_t corb_read_uint16(corb_t *rb, bool *succ);
int32_t corb_read_int32(corb_t *rb, bool *succ);
uint32_t corb_read_uint32(corb_t *rb, bool *succ);
int64_t corb_read_int64(corb_t *rb, bool *succ);
uint64_t corb_read_uint64(corb_t *rb, bool *succ);
float corb_read_float32(corb_t *rb, bool *succ);
double corb_read_float64(corb_t *rb, bool *succ);
bool corb_read_buffer(corb_t *rb, void *buffer, int size);

///////////////////////////////////////////////////////////////////////////////////
// 写buffer：通常用于生成格式化的数据

typedef struct cowb {
    void *buffer;
    int size;
    int pos;
    cb_endian_t endian;
} cowb_t;

// 初始化和释放 
void cowb_init(cowb_t *wb, int initsize, cb_endian_t endian);
void cowb_free(cowb_t *wb);
// 定位: abs指定是否绝对定位 false表示相对于当前pos，绝对定位支持-1,-2表示最后，返回实际的定位
int cowb_seek(cowb_t *wb, bool abs, int pos);
// buff大小位置等
static inline void* cowb_buffer(cowb_t *wb) { return wb->buffer; };
static inline int cowb_size(cowb_t *wb) { return wb->size; }
static inline int cowb_pos(cowb_t *wb) { return wb->pos; }
static inline int cowb_remainder(cowb_t *wb) { return wb->size - wb->pos; }
// 写各种类型
void cowb_write_int8(cowb_t *wb, int8_t v);
void cowb_write_uint8(cowb_t *wb, uint8_t v);
void cowb_write_int16(cowb_t *wb, int16_t v);
void cowb_write_uint16(cowb_t *wb, uint16_t v);
void cowb_write_int32(cowb_t *wb, int32_t v);
void cowb_write_uint32(cowb_t *wb, uint32_t v);
void cowb_write_int64(cowb_t *wb, int64_t v);
void cowb_write_uint64(cowb_t *wb, uint64_t v);
void cowb_write_float32(cowb_t *wb, float v);
void cowb_write_float64(cowb_t *wb, double v);
void cowb_write_buffer(cowb_t *wb, void *buffer, int size);

///////////////////////////////////////////////////////////////////////////////////
// 圆形buffer: 用于生产者/消费者模式下的buffer，自增长

typedef struct cocb {
    void *buffer;
    int size;
    int head;
    int tail;
} cocb_t;

// 初始化和释放circle buffer
void cocb_init(cocb_t *cb, int initsize);
void cocb_free(cocb_t *cb);
// 读buffer，返回实际读到的buffer大小
int cocb_read(cocb_t *cb, void *buffer, int size);
// 写buffer
void cocb_write(cocb_t *cb, const void *buffer, int size);
// 取缓冲的头
void *cocb_head(cocb_t *cb);
// 还剩多少可以读
int cocb_readable_size(cocb_t *cb);
// 可读一次连续的内存的大小
int cocb_readonce_size(cocb_t *cb);
// 消耗了多少内存
bool cocb_consume_size(cocb_t *cb, int size);

#ifdef __cplusplus
}
#endif

#endif