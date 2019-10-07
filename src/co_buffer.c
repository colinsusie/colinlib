#include "co_buffer.h"
#include "co_endian.h"

#define MIN_BUFF_SIZE 32

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

cobuffer_t* cobuffer_new(const char *buffer, int bufsz, cb_endian_t endian) {
    cobuffer_t* bf = CO_MALLOC(sizeof(*bf));
    int size = CO_MAX(bufsz, MIN_BUFF_SIZE);
    bf->buffer = CO_MALLOC(size);
    bf->size = size;
    bf->pos = 0;
    bf->endian = endian;
    if (buffer && bufsz)
        cobuffer_write(bf, buffer, bufsz);
    return bf;
}

void* cobuffer_free(cobuffer_t *bf) {
    CO_FREE(bf->buffer);
    CO_FREE(bf);
    return NULL;
}

int cobuffer_seek(cobuffer_t *bf, bool abs, int pos) {
    if (!abs)
        pos += bf->pos;
    else if (pos < 0)
        pos = bf->size - pos;
    bf->pos = CO_CLAMP(pos, 0, bf->size-1);
    return bf->pos;
}

int8_t cobuffer_read_int8(cobuffer_t *bf, bool *succ) {
    int8_t v;
    *succ = cobuffer_read(bf, &v, sizeof(int8_t));
    return v;
}

uint8_t cobuffer_read_uint8(cobuffer_t *bf, bool *succ) {
    uint8_t v;
    *succ = cobuffer_read(bf, &v, sizeof(uint8_t));
    return v;
}

int16_t cobuffer_read_int16(cobuffer_t *bf, bool *succ) {
    int16_t v;
    *succ = cobuffer_read(bf, &v, sizeof(int16_t));
    if (*succ) {
        if (bf->endian == CB_EN_LITTLE)
            v = le16toh(v);
        else if (bf->endian == CB_EN_BIG)
            v = be16toh(v);
    }
    return v;
}

uint16_t cobuffer_read_uint16(cobuffer_t *bf, bool *succ) {
    uint16_t v;
    *succ = cobuffer_read(bf, &v, sizeof(uint16_t));
    if (*succ) {
        if (bf->endian == CB_EN_LITTLE)
            v = le16toh(v);
        else if (bf->endian == CB_EN_BIG)
            v = be16toh(v);
    }
    return v;
}

int32_t cobuffer_read_int32(cobuffer_t *bf, bool *succ) {
    int32_t v;
    *succ = cobuffer_read(bf, &v, sizeof(int32_t));
    if (*succ) {
        if (bf->endian == CB_EN_LITTLE)
            v = le32toh(v);
        else if (bf->endian == CB_EN_BIG)
            v = be32toh(v);
    }
    return v;
}

uint32_t cobuffer_read_uint32(cobuffer_t *bf, bool *succ) {
    uint32_t v;
    *succ = cobuffer_read(bf, &v, sizeof(uint32_t));
    if (*succ) {
        if (bf->endian == CB_EN_LITTLE)
            v = le32toh(v);
        else if (bf->endian == CB_EN_BIG)
            v = be32toh(v);
    }
    return v;
}

int64_t cobuffer_read_int64(cobuffer_t *bf, bool *succ) {
    int64_t v;
    *succ = cobuffer_read(bf, &v, sizeof(int64_t));
    if (*succ) {
        if (bf->endian == CB_EN_LITTLE)
            v = le64toh(v);
        else if (bf->endian == CB_EN_BIG)
            v = be64toh(v);
    }
    return v;

}

uint64_t cobuffer_read_uint64(cobuffer_t *bf, bool *succ) {
    uint64_t v;
    *succ = cobuffer_read(bf, &v, sizeof(uint64_t));
    if (*succ) {
        if (bf->endian == CB_EN_LITTLE)
            v = le64toh(v);
        else if (bf->endian == CB_EN_BIG)
            v = be64toh(v);
    }
    return v;
}

float cobuffer_read_float32(cobuffer_t *bf, bool *succ) {
    float v;
    *succ = cobuffer_read(bf, &v, sizeof(float));
    if (*succ) {
        if (bf->endian == CB_EN_LITTLE)
            v = le32tohf(v);
        else if (bf->endian == CB_EN_BIG)
            v = be32tohf(v);
    }
    return v;
}

double cobuffer_read_float64(cobuffer_t *bf, bool *succ) {
    double v;
    *succ = cobuffer_read(bf, &v, sizeof(double));
    if (*succ) {
        if (bf->endian == CB_EN_LITTLE)
            v = le64tohf(v);
        else if (bf->endian == CB_EN_BIG)
            v = be64tohf(v);
    }
    return v;
}

bool cobuffer_read(cobuffer_t *bf, void *buffer, int size) {
    if (bf->pos + size <= bf->size) {
        memcpy(buffer, (char*)bf->buffer+bf->pos, size);
        bf->pos += size;
        return true;
    }
    return false;
}

static inline void _wb_check_and_grow(cobuffer_t *bf, int size) {
    if (bf->pos + size > bf->size) {
        int newsize = CO_MAX(bf->pos + size, bf->size * 2);
        bf->buffer = CO_REALLOC(bf->buffer, newsize);
        bf->size = newsize;
    }
}

void cobuffer_write_int8(cobuffer_t *bf, int8_t v) {
    cobuffer_write(bf, &v, sizeof(int8_t));
}

void cobuffer_write_uint8(cobuffer_t *bf, uint8_t v) {
    cobuffer_write(bf, &v, sizeof(uint8_t));
}

void cobuffer_write_int16(cobuffer_t *bf, int16_t v) {
    if (bf->endian == CB_EN_LITTLE)
        v = htole16(v);
    else if (bf->endian == CB_EN_BIG)
        v = htobe16(v);
    cobuffer_write(bf, &v, sizeof(int16_t));
}

void cobuffer_write_uint16(cobuffer_t *bf, uint16_t v) {
    if (bf->endian == CB_EN_LITTLE)
        v = htole16(v);
    else if (bf->endian == CB_EN_BIG)
        v = htobe16(v);
    cobuffer_write(bf, &v, sizeof(uint16_t));
}

void cobuffer_write_int32(cobuffer_t *bf, int32_t v) {
    if (bf->endian == CB_EN_LITTLE)
        v = htole32(v);
    else if (bf->endian == CB_EN_BIG)
        v = htobe32(v);
    cobuffer_write(bf, &v, sizeof(int32_t));
}

void cobuffer_write_uint32(cobuffer_t *bf, uint32_t v) {
    if (bf->endian == CB_EN_LITTLE)
        v = htole32(v);
    else if (bf->endian == CB_EN_BIG)
        v = htobe32(v);
    cobuffer_write(bf, &v, sizeof(uint32_t));
}

void cobuffer_write_int64(cobuffer_t *bf, int64_t v) {
    if (bf->endian == CB_EN_LITTLE)
        v = htole64(v);
    else if (bf->endian == CB_EN_BIG)
        v = htobe64(v);
    cobuffer_write(bf, &v, sizeof(int64_t));
}

void cobuffer_write_uint64(cobuffer_t *bf, uint64_t v) {
    if (bf->endian == CB_EN_LITTLE)
        v = htole64(v);
    else if (bf->endian == CB_EN_BIG)
        v = htobe64(v);
    cobuffer_write(bf, &v, sizeof(uint64_t));
}

void cobuffer_write_float32(cobuffer_t *bf, float v) {
    if (bf->endian == CB_EN_LITTLE)
        v = htole32f(v);
    else if (bf->endian == CB_EN_BIG)
        v = htobe32f(v);
    cobuffer_write(bf, &v, sizeof(float));
}

void cobuffer_write_float64(cobuffer_t *bf, double v) {
    if (bf->endian == CB_EN_LITTLE)
        v = htole64f(v);
    else if (bf->endian == CB_EN_BIG)
        v = htobe64f(v);
    cobuffer_write(bf, &v, sizeof(double));
}

void cobuffer_write(cobuffer_t *bf, const void *buffer, int size) {
    _wb_check_and_grow(bf, size);
    memcpy((char*)bf->buffer+bf->pos, buffer, size);
    bf->pos += size;
}

coringbuf_t* coringbuf_new(int initsize) {
    coringbuf_t *rb = CO_MALLOC(sizeof(*rb));
    rb->size = initsize;
    rb->head = 0;
    rb->tail = 0;
    rb->buffer = CO_MALLOC(initsize);
    return rb;
}

void* coringbuf_free(coringbuf_t *rb) {
    CO_FREE(rb->buffer);
    CO_FREE(rb);
    return NULL;
}

int coringbuf_read(coringbuf_t *rb, void *buffer, int size) {
    int readable = coringbuf_readable_size(rb);
    size = readable >= size ? size : readable;
    int readonce = coringbuf_readonce_size(rb);
    int len = CO_MIN(size, readonce);
    memcpy(buffer, (uint8_t*)rb->buffer + rb->head, len);
    if (size > len)
        memcpy((uint8_t*)buffer + len, rb->buffer, size-len);
    rb->head = (rb->head + size) % rb->size;
    return size;
}

static inline void _cb_check_and_grow(coringbuf_t *rb, int size) {
    int readable = coringbuf_readable_size(rb);
    int writable = rb->size - readable;
    if (writable <= size) {
        int newsize = CO_MAX(rb->size * 2, readable + size + 1);
        rb->buffer = CO_REALLOC(rb->buffer, newsize);
        if (rb->tail < rb->head) {
            int readonce = coringbuf_readonce_size(rb);
            memmove((uint8_t*)rb->buffer + newsize - readonce, (uint8_t*)rb->buffer + rb->head, readonce);
            rb->head = newsize - readonce;
        }
        rb->size = newsize;
    }
}

void coringbuf_write(coringbuf_t *rb, const void *buffer, int size) {
    _cb_check_and_grow(rb, size);
    int len = CO_MIN(size, rb->size - rb->tail);
    memcpy((uint8_t*)rb->buffer + rb->tail, buffer, len);
    if (size > len)
        memcpy((uint8_t*)rb->buffer, (uint8_t*)buffer + len, size - len);
    rb->tail = (rb->tail + size) % rb->size;
}

void *coringbuf_head(coringbuf_t *rb) {
    return (uint8_t*)rb->buffer + rb->head;
}

int coringbuf_readable_size(coringbuf_t *rb) {
    if (rb->tail >= rb->head)
        return rb->tail - rb->head;
    else
        return rb->size - (rb->head - rb->tail);
}

int coringbuf_readonce_size(coringbuf_t *rb) {
    if (rb->tail >= rb->head)
        return rb->tail - rb->head;
    else
        return rb->size - rb->head;
}

bool coringbuf_consume_size(coringbuf_t *rb, int size) {
    int readable = coringbuf_readable_size(rb);
    if (readable >= size) {
        rb->head = (rb->head + size) % rb->size;
        return true;
    }
    return false;
}