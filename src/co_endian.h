/**
 * 字节序转换
 */
#ifndef __CO_ENDIAN__
#define __CO_ENDIAN__

#ifdef __cplusplus
extern "C" {
#endif

// 判断是否是小端字节序
static inline bool is_little_endian() {
    static const union {
        int dummy;
        char little;
    } host_endian = {1};
    return host_endian.little;
}

#if defined(__linux__)
    #include <endian.h>
#elif defined(__APPLE__)
    #include <libkern/OSByteOrder.h>
    #define htobe16(x) OSSwapHostToBigInt16(x)
    #define htole16(x) OSSwapHostToLittleInt16(x)
    #define be16toh(x) OSSwapBigToHostInt16(x)
    #define le16toh(x) OSSwapLittleToHostInt16(x)

    #define htobe32(x) OSSwapHostToBigInt32(x)
    #define htole32(x) OSSwapHostToLittleInt32(x)
    #define be32toh(x) OSSwapBigToHostInt32(x)
    #define le32toh(x) OSSwapLittleToHostInt32(x)
 
    #define htobe64(x) OSSwapHostToBigInt64(x)
    #define htole64(x) OSSwapHostToLittleInt64(x)
    #define be64toh(x) OSSwapBigToHostInt64(x)
    #define le64toh(x) OSSwapLittleToHostInt64(x)
#elif defined(__OpenBSD__)
    #include <sys/endian.h>
#elif defined(__NetBSD__) || defined(__FreeBSD__)
    #include <sys/endian.h>
    #define be16toh(x) betoh16(x)
    #define le16toh(x) letoh16(x)
    #define be32toh(x) betoh32(x)
    #define le32toh(x) letoh32(x)
    #define be64toh(x) betoh64(x)
    #define le64toh(x) letoh64(x)
#else
    #error platform not supported
#endif

typedef union floattype {
  float f;
  double d;
  char buff[8];
} floattype_t;

static inline void _swap_float(char *dest, const char *src, int size) {
    dest += size - 1;
    while (size-- != 0)
      *(dest--) = *(src++);
}

static inline float htole32f(float v) {
    if (is_little_endian()) {
        return v;
    } else {
        floattype_t f;
        _swap_float(f.buff, (char*)&v, sizeof(float));
        return f.f; 
    }
}

static inline float htobe32f(float v) {
    if (!is_little_endian()) {
        return v;
    } else {
        floattype_t f;
        _swap_float(f.buff, (char*)&v, sizeof(float));
        return f.f; 
    }
}

static inline double htole64f(double v) {
    if (is_little_endian()) {
        return v;
    } else {
        floattype_t f;
        _swap_float(f.buff, (char*)&v, sizeof(double));
        return f.d; 
    }
}

static inline double htobe64f(double v) {
    if (!is_little_endian()) {
        return v;
    } else {
        floattype_t f;
        _swap_float(f.buff, (char*)&v, sizeof(double));
        return f.d; 
    }
}

static inline float le32tohf(float v) {
    return htole32f(v);
}

static inline double be32tohf(double v) {
    return htobe32f(v);
}

static inline float le64tohf(float v) {
    return htole64f(v);
}

static inline double be64tohf(double v) {
    return htobe64f(v);
}


#ifdef __cplusplus
}
#endif
#endif
