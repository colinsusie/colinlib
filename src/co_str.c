#include "co_str.h"

cochar* costr_new(const char *s, int len) {
    int sz = len >= 0 ? len+1 : strlen(s)+1;
    int cap = sz + sizeof(costrheader_t);
    costrheader_t *header = CO_CALLOC(cap, 1);
    header->len = sz;
    header->cap = cap;
    cochar *str = (cochar*)header + sizeof(costrheader_t);
    if (s) memcpy(str, s, sz);
    return str;
}

void costr_free(const char *s) {
    CO_FREE(s-sizeof(costrheader_t));
}

cochar* costr_format(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    cochar *res = costr_formatv(fmt, ap);
    va_end(ap);
    return res;
}

cochar* costr_formatv(const char *fmt, va_list ap) {
    va_list cpy;
    int maxsz = 256;
    while (1) {
        cochar *costr = costr_new(NULL, maxsz);
        va_copy(cpy,ap);
        int len = vsnprintf(costr, maxsz, fmt, cpy);
        va_end(cpy);
        if (len < 0) {
            costr_free(costr);
            return NULL;
        } else if (len >= maxsz) {
            costr_free(costr);
            maxsz *= 2;
        } else {
            return costr;
        }
    }
}
