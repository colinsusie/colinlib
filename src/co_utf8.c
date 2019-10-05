#include "co_utf8.h"

#define MAXUNICODE  0x10FFFF
static const unsigned int limits[] = {0xFF, 0x7F, 0x7FF, 0xFFFF};

const char *coutf8_decode(const char *s, int *code) {
    const uint8_t *b = (const uint8_t *)s;
    uint32_t ch = b[0];
    uint32_t res = 0;
    if (ch < 0x80) {
        res = ch;
    } else {
        int count = 0;
        while (ch & 0x40) {
            int cc = b[++count];
            if ((cc & 0xc0) != 0x80) return NULL;
            res = (res << 6) | (cc & 0x3f);
            ch <<= 1;
        }
        res |= ((ch & 0x7f) << (count * 5));
        if (count > 3 || res > MAXUNICODE || res <= limits[count])
          return NULL;
        b += count;
    }
    if (code)
        *code = res;
    return (const char *)b + 1;
}

int coutf8_len(const char *s) {
    int count = 0;
    const char *p = s;
    while (*p) {
        p = coutf8_decode(p, NULL);
        if (!p) break;
        ++count;
    }
    return count;
}