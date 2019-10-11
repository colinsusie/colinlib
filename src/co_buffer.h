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

// 打印BUFF：调试用
void coprintfbuffer(const void *buffer, int size, int blocksize);

///////////////////////////////////////////////////////////////////////////////////
// 读写buffer

typedef struct cobuffer {
    void *buffer;           // 内存
    int size;               // 内存大小
    int pos;                // 当前读或写的位置
    cb_endian_t endian;     // 字节序
} cobuffer_t;

// 初始化和释放 
cobuffer_t* cobuffer_new(const char *buffer, int bufsz, cb_endian_t endian);
void* cobuffer_free(cobuffer_t *bf);
// 定位: abs指定是否绝对定位 false表示相对于当前pos，绝对定位支持-1,-2表示最后，返回实际的定位
int cobuffer_seek(cobuffer_t *bf, bool abs, int pos);
// buff大小位置等
static inline void* cobuffer_buffer(cobuffer_t *bf) { return bf->buffer; }
static inline int cobuffer_size(cobuffer_t *bf) { return bf->size; }
static inline int cobuffer_pos(cobuffer_t *bf) { return bf->pos; }
// 通用读写
bool cobuffer_read(cobuffer_t *bf, void *buffer, int size);
void cobuffer_write(cobuffer_t *bf, const void *buffer, int size);
// 读各种类型
int8_t cobuffer_read_int8(cobuffer_t *bf, bool *succ);
uint8_t cobuffer_read_uint8(cobuffer_t *bf, bool *succ);
int16_t cobuffer_read_int16(cobuffer_t *bf, bool *succ);
uint16_t cobuffer_read_uint16(cobuffer_t *bf, bool *succ);
int32_t cobuffer_read_int32(cobuffer_t *bf, bool *succ);
uint32_t cobuffer_read_uint32(cobuffer_t *bf, bool *succ);
int64_t cobuffer_read_int64(cobuffer_t *bf, bool *succ);
uint64_t cobuffer_read_uint64(cobuffer_t *bf, bool *succ);
float cobuffer_read_float32(cobuffer_t *bf, bool *succ);
double cobuffer_read_float64(cobuffer_t *bf, bool *succ);
// 写各种类型
void cobuffer_write_int8(cobuffer_t *bf, int8_t v);
void cobuffer_write_uint8(cobuffer_t *bf, uint8_t v);
void cobuffer_write_int16(cobuffer_t *bf, int16_t v);
void cobuffer_write_uint16(cobuffer_t *bf, uint16_t v);
void cobuffer_write_int32(cobuffer_t *bf, int32_t v);
void cobuffer_write_uint32(cobuffer_t *bf, uint32_t v);
void cobuffer_write_int64(cobuffer_t *bf, int64_t v);
void cobuffer_write_uint64(cobuffer_t *bf, uint64_t v);
void cobuffer_write_float32(cobuffer_t *bf, float v);
void cobuffer_write_float64(cobuffer_t *bf, double v);


///////////////////////////////////////////////////////////////////////////////////
// 圆形buffer: 用于生产者/消费者模式下的buffer，自增长

typedef struct coringbuf {
    void *buffer;
    int size;
    int head;
    int tail;
} coringbuf_t;

// 初始化和释放circle buffer
coringbuf_t* coringbuf_new(int initsize);
void* coringbuf_free(coringbuf_t *rb);
// 读buffer，返回实际读到的buffer大小
int coringbuf_read(coringbuf_t *rb, void *buffer, int size);
// 写buffer
void coringbuf_write(coringbuf_t *rb, const void *buffer, int size);
// 取缓冲的头
void *coringbuf_head(coringbuf_t *rb);
// 还剩多少可以读
int coringbuf_readable_size(coringbuf_t *rb);
// 可读一次连续的内存的大小
int coringbuf_readonce_size(coringbuf_t *rb);
// 消耗了多少内存
bool coringbuf_consume_size(coringbuf_t *rb, int size);

#ifdef __cplusplus
}
#endif

#endif