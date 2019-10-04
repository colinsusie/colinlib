/**
 * 各种buffer
 *                  by colin
 */
#ifndef __CO_BUFFER__
#define __CO_BUFFER__
#include "co_utils.h"

typedef enum cb_endian {
    CB_EN_NATIVE,
    CB_EN_LITTLE,
    CB_EN_BIG,
} cb_endian_t;

// 只读buffer
typedef struct corb {
    uint8_t *buffer;
    int pos;
    int size;
} corb_t;

void corb_attach(corb_t *rb, void *buffer, int size);
int corb_seek(corb_t *rb, bool abs, int pos);
int8_t corb_read_int8(corb_t *rb, bool *succ);
uint8_t corb_read_uint8(corb_t *rb, bool *succ);
int16_t corb_read_int16(corb_t *rb, cb_endian_t endian, bool *succ);
uint16_t corb_read_uint16(corb_t *rb, cb_endian_t endian, bool *succ);
int32_t corb_read_int32(corb_t *rb, cb_endian_t endian, bool *succ);
uint32_t corb_read_uint32(corb_t *rb, cb_endian_t endian, bool *succ);
int64_t corb_read_int64(corb_t *rb, cb_endian_t endian, bool *succ);
uint64_t corb_read_uint64(corb_t *rb, cb_endian_t endian, bool *succ);
float corb_read_float32(corb_t *rb, cb_endian_t endian, bool *succ);
double corb_read_float64(corb_t *rb, cb_endian_t endian, bool *succ);
bool corb_read_buffer(corb_t *rb, void *buffer, int size);



#endif