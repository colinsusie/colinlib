/**
 * 字节序转换
 */
#ifndef __CO_ENDIAN__
#define __CO_ENDIAN__

#if defined(__linux__) || defined(__CYGWIN__)
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

    #define __BYTE_ORDER    BYTE_ORDER
    #define __BIG_ENDIAN    BIG_ENDIAN
    #define __LITTLE_ENDIAN LITTLE_ENDIAN
    #define __PDP_ENDIAN    PDP_ENDIAN
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

#endif
