#include "../src/co_utf8.h"

void print_utf8_codepoint(const char *s) {
    int code;
    const char *p = s;
    while (*p) {
        p = coutf8_decode(p, &code);
        if (!p) {
            printf("decode error\n");
            break;
        }
        printf("U+%04X ", code);
    }
    printf("\n");
}

int main(int argc, char const *argv[])
{
    char *s = "你好hello世界";
    print_utf8_codepoint(s);
    printf("len=%d\n", coutf8_len(s));
    return 0;
}
