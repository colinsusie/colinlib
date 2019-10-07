#include "../src/co_buffer.h"

void test_endian() {
    cobuffer_t *wb = cobuffer_new(NULL, 16, CB_EN_BIG);
    cobuffer_write_int8(wb, 0x10);
    cobuffer_write_int8(wb, 0x11);
    cobuffer_write_int8(wb, 0x12);
    cobuffer_write_int8(wb, 0x13);
    coprintfbuffer(wb->buffer, wb->pos, 1);

    cobuffer_seek(wb, true, 0);
    cobuffer_write_int16(wb, 0xFE01);
    cobuffer_write_uint16(wb, 0xFE02);
    cobuffer_write_int32(wb, 0xFE03);
    cobuffer_write_uint32(wb, 0xFE04);
    cobuffer_write_int64(wb, 0xFE05);
    cobuffer_write_uint64(wb, 0xFE06);
    cobuffer_write_float32(wb, 1.2345f);
    cobuffer_write_float64(wb, 1.2345);
    coprintfbuffer(wb->buffer, wb->pos, 2);
    cobuffer_free(wb);

    wb = cobuffer_new(NULL, 16, CB_EN_LITTLE);
    cobuffer_write_int16(wb, 0xFE01);
    cobuffer_write_uint16(wb, 0xFE02);
    cobuffer_write_int32(wb, 0xFE03);
    cobuffer_write_uint32(wb, 0xFE04);
    cobuffer_write_int64(wb, 0xFE05);
    cobuffer_write_uint64(wb, 0xFE06);
    cobuffer_write_float32(wb, 1.2345f);
    cobuffer_write_float64(wb, 1.2345);
    coprintfbuffer(wb->buffer, wb->pos, 2);
    cobuffer_free(wb);

    wb = cobuffer_new(NULL, 16, CB_EN_NATIVE);
    cobuffer_write_int16(wb, 0xFE01);
    cobuffer_write_uint16(wb, 0xFE02);
    cobuffer_write_int32(wb, 0xFE03);
    cobuffer_write_uint32(wb, 0xFE04);
    cobuffer_write_int64(wb, 0xFE05);
    cobuffer_write_uint64(wb, 0xFE06);
    cobuffer_write_float32(wb, 1.2345f);
    cobuffer_write_float64(wb, 1.2345);
    coprintfbuffer(wb->buffer, wb->pos, 2);
    cobuffer_free(wb);
}

void test_read_write() {
    struct mydata {
        int d1;
        float f1;
        int64_t d2;
    };
    struct mydata md = {0x11FE, 1.2211, 0x32EDD1112};

    cobuffer_t *wb = cobuffer_new(NULL, 16, CB_EN_BIG);
    cobuffer_write_int64(wb, 0xFE05);
    cobuffer_write_uint64(wb, 0xFE06);
    cobuffer_write_float32(wb, 1.2345f);
    cobuffer_write_float64(wb, 1.2345);
    cobuffer_write(wb, &md, sizeof(md));
    coprintfbuffer(wb->buffer, wb->pos, 2);

    bool succ;
    cobuffer_seek(wb, true, 0);
    printf("%jX\n", cobuffer_read_int64(wb, &succ));
    printf("%jX\n", cobuffer_read_uint64(wb, &succ));
    printf("%f\n", cobuffer_read_float32(wb, &succ));
    printf("%f\n", cobuffer_read_float64(wb, &succ));
    struct mydata md2;
    cobuffer_read(wb, &md2, sizeof(md2));
    printf("d1=%X\n", md2.d1);
    printf("f1=%f\n", md2.f1);
    printf("d2=%jX\n", md2.d2);

    cobuffer_free(wb);
}

void test_circle_buffer() {
    coringbuf_t *rb = coringbuf_new(64);

    char *str = "helloworldhelloworldhelloworldhelloworldhelloworld";
    int len = strlen(str);
    coringbuf_write(rb, str, len);
    coringbuf_write(rb, str, len);
    coringbuf_write(rb, str, len);

    char str2[19] = {0};
    while (1) {
        int n = coringbuf_read(rb, str2, 18);
        printf(">>>>>>%s\n", str2);
        if (n < 18) break;
    }

    coringbuf_free(rb);
}

void test_circle_buffer2() {
    coringbuf_t *rb = coringbuf_new(64);
    char *str = "helloworldhelloworldhelloworldhelloworldhelloworld";
    int len = strlen(str);
    char str2[50] = {0};
    coringbuf_write(rb, str, len);
    coringbuf_write(rb, str, len);
    coringbuf_read(rb, str2, 50);
    coringbuf_read(rb, str2, 50);
    coringbuf_write(rb, str, len);

    printf("readable=%d, head=%d, tail=%d\n", coringbuf_readable_size(rb), rb->head, rb->tail);

    while (coringbuf_readable_size(rb)) {
        int readonce = coringbuf_readonce_size(rb);
        void *buffer = coringbuf_head(rb);
        // 比如用于socket write
        (void)buffer;
        // comsume buffer ...
        printf("comsume buffer: %d\n", readonce);
        coringbuf_consume_size(rb, readonce);
    }

    coringbuf_free(rb);
}

int main(int argc, char const *argv[])
{
    test_endian();
    test_read_write();
    test_circle_buffer();
    test_circle_buffer2();
    return 0;
}
