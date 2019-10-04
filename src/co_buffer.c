#include "co_buffer.h"

void corb_attach(corb_t *rb, void *buffer, int size) {
    rb->buffer = (uint8_t*)buffer;
    rb->size = size;
    rb->pos = 0;
}

int corb_seek(corb_t *rb, bool abs, int pos) {
    if (!abs)
        pos += rb->pos;
    else if (pos < 0)
        pos = rb->size - pos;
    if (pos >= 0 && pos < rb->size)
        rb->pos = pos;
    return rb->pos;
}

int8_t corb_read_int8(corb_t *rb, bool *succ) {
    int8_t v;
    *succ = corb_read_buffer(rb, &v, sizeof(int8_t));
    return v;
}

uint8_t corb_read_uint8(corb_t *rb, bool *succ) {
    uint8_t v;
    *succ = corb_read_buffer(rb, &v, sizeof(uint8_t));
    return v;
}

int16_t corb_read_int16(corb_t *rb, cb_endian_t endian, bool *succ) {
    int16_t v;
    *succ = corb_read_buffer(rb, &v, sizeof(int16_t));
    if (*succ) {
        if (endian == CB_EN_BIG)
            v = htobe16(v);
        else if (endian == CB_EN_LITTLE)
            v = htole16(v);
    }
    return v;
}

uint16_t corb_read_uint16(corb_t *rb, cb_endian_t endian, bool *succ) {
    uint16_t v;
    *succ = corb_read_buffer(rb, &v, sizeof(uint16_t));
    if (*succ) {
        if (endian == CB_EN_BIG)
            v = htobe16(v);
        else if (endian == CB_EN_LITTLE)
            v = htole16(v);
    }
    return v;
}

// int32_t corb_read_int32(corb_t *rb, cb_endian_t endian, bool *succ);
// uint32_t corb_read_uint32(corb_t *rb, cb_endian_t endian, bool *succ);
// int64_t corb_read_int64(corb_t *rb, cb_endian_t endian, bool *succ);
// uint64_t corb_read_uint64(corb_t *rb, cb_endian_t endian, bool *succ);
// float corb_read_float32(corb_t *rb, cb_endian_t endian, bool *succ);
// double corb_read_float64(corb_t *rb, cb_endian_t endian, bool *succ);

bool corb_read_buffer(corb_t *rb, void *buffer, int size) {
    if (rb->pos + size < rb->size) {
        memcpy(buffer, rb->buffer, size);
        rb->pos += size;
        return true;
    }
    return false;
}