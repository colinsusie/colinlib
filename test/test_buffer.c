#include "../src/co_buffer.h"

void test_endian() {
    cowb_t wb;
    cowb_init(&wb, 16, CB_EN_BIG);
    cowb_write_int8(&wb, 0x10);
    cowb_write_int8(&wb, 0x11);
    cowb_write_int8(&wb, 0x12);
    cowb_write_int8(&wb, 0x13);
    coprintfbuffer(wb.buffer, wb.pos, 1);

    cowb_seek(&wb, true, 0);
    cowb_write_int16(&wb, 0xFE01);
    cowb_write_uint16(&wb, 0xFE02);
    cowb_write_int32(&wb, 0xFE03);
    cowb_write_uint32(&wb, 0xFE04);
    cowb_write_int64(&wb, 0xFE05);
    cowb_write_uint64(&wb, 0xFE06);
    cowb_write_float32(&wb, 1.2345f);
    cowb_write_float64(&wb, 1.2345);
    coprintfbuffer(wb.buffer, wb.pos, 2);
    cowb_free(&wb);

    cowb_init(&wb, 16, CB_EN_LITTLE);
    cowb_write_int16(&wb, 0xFE01);
    cowb_write_uint16(&wb, 0xFE02);
    cowb_write_int32(&wb, 0xFE03);
    cowb_write_uint32(&wb, 0xFE04);
    cowb_write_int64(&wb, 0xFE05);
    cowb_write_uint64(&wb, 0xFE06);
    cowb_write_float32(&wb, 1.2345f);
    cowb_write_float64(&wb, 1.2345);
    coprintfbuffer(wb.buffer, wb.pos, 2);
    cowb_free(&wb);

    cowb_init(&wb, 16, CB_EN_NATIVE);
    cowb_write_int16(&wb, 0xFE01);
    cowb_write_uint16(&wb, 0xFE02);
    cowb_write_int32(&wb, 0xFE03);
    cowb_write_uint32(&wb, 0xFE04);
    cowb_write_int64(&wb, 0xFE05);
    cowb_write_uint64(&wb, 0xFE06);
    cowb_write_float32(&wb, 1.2345f);
    cowb_write_float64(&wb, 1.2345);
    coprintfbuffer(wb.buffer, wb.pos, 2);
    cowb_free(&wb);
}

void test_read_write() {
    struct mydata {
        int d1;
        float f1;
        int64_t d2;
    };
    struct mydata md = {0x11FE, 1.2211, 0x32EDD1112};

    cowb_t wb;
    cowb_init(&wb, 16, CB_EN_BIG);
    cowb_write_int64(&wb, 0xFE05);
    cowb_write_uint64(&wb, 0xFE06);
    cowb_write_float32(&wb, 1.2345f);
    cowb_write_float64(&wb, 1.2345);
    cowb_write_buffer(&wb, &md, sizeof(md));
    coprintfbuffer(wb.buffer, wb.pos, 2);

    bool succ;
    corb_t rb;
    corb_attach(&rb, wb.buffer, wb.size, CB_EN_BIG);
    printf("%jX\n", corb_read_int64(&rb, &succ));
    printf("%jX\n", corb_read_uint64(&rb, &succ));
    printf("%f\n", corb_read_float32(&rb, &succ));
    printf("%f\n", corb_read_float64(&rb, &succ));
    struct mydata md2;
    corb_read_buffer(&rb, &md2, sizeof(md2));
    printf("d1=%X\n", md2.d1);
    printf("f1=%f\n", md2.f1);
    printf("d2=%jX\n", md2.d2);

    cowb_free(&wb);
}

void test_circle_buffer() {
    cocb_t cb;
    cocb_init(&cb, 64);

    char *str = "helloworldhelloworldhelloworldhelloworldhelloworld";
    int len = strlen(str);
    cocb_write(&cb, str, len);
    cocb_write(&cb, str, len);
    cocb_write(&cb, str, len);

    char str2[19] = {0};
    while (1) {
        int n = cocb_read(&cb, str2, 18);
        printf(">>>>>>%s\n", str2);
        if (n < 18) break;
    }

    cocb_free(&cb);
}

void test_circle_buffer2() {
    cocb_t cb;
    cocb_init(&cb, 64);
    char *str = "helloworldhelloworldhelloworldhelloworldhelloworld";
    int len = strlen(str);
    char str2[50] = {0};
    cocb_write(&cb, str, len);
    cocb_write(&cb, str, len);
    cocb_read(&cb, str2, 50);
    cocb_read(&cb, str2, 50);
    cocb_write(&cb, str, len);

    printf("readable=%d, head=%d, tail=%d\n", cocb_readable_size(&cb), cb.head, cb.tail);

    while (cocb_readable_size(&cb)) {
        int readonce = cocb_readonce_size(&cb);
        void *buffer = cocb_head(&cb);
        // 比如用于socket write
        (void)buffer;
        // comsume buffer ...
        printf("comsume buffer: %d\n", readonce);
        cocb_consume_size(&cb, readonce);
    }

    cocb_free(&cb);
}

int main(int argc, char const *argv[])
{
    test_endian();
    test_read_write();
    test_circle_buffer();
    test_circle_buffer2();
    return 0;
}
