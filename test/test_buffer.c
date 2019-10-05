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

int main(int argc, char const *argv[])
{
    test_endian();
    test_read_write();
    return 0;
}
