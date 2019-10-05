#include "co_buffer.h"
#include "co_endian.h"

void coprintfbuffer(const void *buffer, int size, int blocksize) {
    int i, j;
    int p = 0;
    const uint8_t *s = (const uint8_t*)buffer;
    while (1) {
        for (i = 0; i < 80; ++i) {
            for (j = 0; j < blocksize; ++j) {
                if (p >= size) {
                    printf("\n");
                    return;
                }
                printf("%.2X", s[p++]);
            }
            printf(" ");
        }
        printf("\n");
    }
}

void corb_attach(corb_t *rb, void *buffer, int size, cb_endian_t endian) {
    rb->buffer = buffer;
    rb->size = size;
    rb->pos = 0;
    rb->endian = endian;
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

int16_t corb_read_int16(corb_t *rb, bool *succ) {
    int16_t v;
    *succ = corb_read_buffer(rb, &v, sizeof(int16_t));
    if (*succ) {
        if (rb->endian == CB_EN_LITTLE)
            v = le16toh(v);
        else if (rb->endian == CB_EN_BIG)
            v = be16toh(v);
    }
    return v;
}

uint16_t corb_read_uint16(corb_t *rb, bool *succ) {
    uint16_t v;
    *succ = corb_read_buffer(rb, &v, sizeof(uint16_t));
    if (*succ) {
        if (rb->endian == CB_EN_LITTLE)
            v = le16toh(v);
        else if (rb->endian == CB_EN_BIG)
            v = be16toh(v);
    }
    return v;
}

int32_t corb_read_int32(corb_t *rb, bool *succ) {
    int32_t v;
    *succ = corb_read_buffer(rb, &v, sizeof(int32_t));
    if (*succ) {
        if (rb->endian == CB_EN_LITTLE)
            v = le32toh(v);
        else if (rb->endian == CB_EN_BIG)
            v = be32toh(v);
    }
    return v;
}

uint32_t corb_read_uint32(corb_t *rb, bool *succ) {
    uint32_t v;
    *succ = corb_read_buffer(rb, &v, sizeof(uint32_t));
    if (*succ) {
        if (rb->endian == CB_EN_LITTLE)
            v = le32toh(v);
        else if (rb->endian == CB_EN_BIG)
            v = be32toh(v);
    }
    return v;
}

int64_t corb_read_int64(corb_t *rb, bool *succ) {
    int64_t v;
    *succ = corb_read_buffer(rb, &v, sizeof(int64_t));
    if (*succ) {
        if (rb->endian == CB_EN_LITTLE)
            v = le64toh(v);
        else if (rb->endian == CB_EN_BIG)
            v = be64toh(v);
    }
    return v;

}

uint64_t corb_read_uint64(corb_t *rb, bool *succ) {
    uint64_t v;
    *succ = corb_read_buffer(rb, &v, sizeof(uint64_t));
    if (*succ) {
        if (rb->endian == CB_EN_LITTLE)
            v = le64toh(v);
        else if (rb->endian == CB_EN_BIG)
            v = be64toh(v);
    }
    return v;
}

float corb_read_float32(corb_t *rb, bool *succ) {
    float v;
    *succ = corb_read_buffer(rb, &v, sizeof(float));
    if (*succ) {
        if (rb->endian == CB_EN_LITTLE)
            v = le32tohf(v);
        else if (rb->endian == CB_EN_BIG)
            v = be32tohf(v);
    }
    return v;
}

double corb_read_float64(corb_t *rb, bool *succ) {
    double v;
    *succ = corb_read_buffer(rb, &v, sizeof(double));
    if (*succ) {
        if (rb->endian == CB_EN_LITTLE)
            v = le64tohf(v);
        else if (rb->endian == CB_EN_BIG)
            v = be64tohf(v);
    }
    return v;
}

bool corb_read_buffer(corb_t *rb, void *buffer, int size) {
    if (rb->pos + size <= rb->size) {
        memcpy(buffer, (char*)rb->buffer+rb->pos, size);
        rb->pos += size;
        return true;
    }
    return false;
}

void cowb_init(cowb_t *wb, int initsize, cb_endian_t endian) {
    assert(initsize > 0);
    wb->buffer = malloc(initsize);
    wb->size = initsize;
    wb->pos = 0;
    wb->endian = endian;
}

void cowb_free(cowb_t *wb) {
    free(wb->buffer);
}

int cowb_seek(cowb_t *wb, bool abs, int pos) {
    if (!abs)
        pos += wb->pos;
    else if (pos < 0)
        pos = wb->size - pos;
    if (pos >= 0 && pos < wb->size)
        wb->pos = pos;
    return wb->pos;
}

static inline void _check_and_grow(cowb_t *wb, int size) {
    if (wb->pos + size > wb->size) {
        int newsize = CO_MAX(wb->pos + size, wb->size * 2);
        wb->buffer = realloc(wb->buffer, newsize);
        wb->size = newsize;
    }
}

void cowb_write_int8(cowb_t *wb, int8_t v) {
    cowb_write_buffer(wb, &v, sizeof(int8_t));
}

void cowb_write_uint8(cowb_t *wb, uint8_t v) {
    cowb_write_buffer(wb, &v, sizeof(uint8_t));
}

void cowb_write_int16(cowb_t *wb, int16_t v) {
    if (wb->endian == CB_EN_LITTLE)
        v = htole16(v);
    else if (wb->endian == CB_EN_BIG)
        v = htobe16(v);
    cowb_write_buffer(wb, &v, sizeof(int16_t));
}

void cowb_write_uint16(cowb_t *wb, uint16_t v) {
    if (wb->endian == CB_EN_LITTLE)
        v = htole16(v);
    else if (wb->endian == CB_EN_BIG)
        v = htobe16(v);
    cowb_write_buffer(wb, &v, sizeof(uint16_t));
}

void cowb_write_int32(cowb_t *wb, int32_t v) {
    if (wb->endian == CB_EN_LITTLE)
        v = htole32(v);
    else if (wb->endian == CB_EN_BIG)
        v = htobe32(v);
    cowb_write_buffer(wb, &v, sizeof(int32_t));
}

void cowb_write_uint32(cowb_t *wb, uint32_t v) {
    if (wb->endian == CB_EN_LITTLE)
        v = htole32(v);
    else if (wb->endian == CB_EN_BIG)
        v = htobe32(v);
    cowb_write_buffer(wb, &v, sizeof(uint32_t));
}

void cowb_write_int64(cowb_t *wb, int64_t v) {
    if (wb->endian == CB_EN_LITTLE)
        v = htole64(v);
    else if (wb->endian == CB_EN_BIG)
        v = htobe64(v);
    cowb_write_buffer(wb, &v, sizeof(int64_t));
}

void cowb_write_uint64(cowb_t *wb, uint64_t v) {
    if (wb->endian == CB_EN_LITTLE)
        v = htole64(v);
    else if (wb->endian == CB_EN_BIG)
        v = htobe64(v);
    cowb_write_buffer(wb, &v, sizeof(uint64_t));
}

void cowb_write_float32(cowb_t *wb, float v) {
    if (wb->endian == CB_EN_LITTLE)
        v = htole32f(v);
    else if (wb->endian == CB_EN_BIG)
        v = htobe32f(v);
    cowb_write_buffer(wb, &v, sizeof(float));
}

void cowb_write_float64(cowb_t *wb, double v) {
    if (wb->endian == CB_EN_LITTLE)
        v = htole64f(v);
    else if (wb->endian == CB_EN_BIG)
        v = htobe64f(v);
    cowb_write_buffer(wb, &v, sizeof(double));
}

void cowb_write_buffer(cowb_t *wb, void *buffer, int size) {
    _check_and_grow(wb, size);
    memcpy((char*)wb->buffer+wb->pos, buffer, size);
    wb->pos += size;
}