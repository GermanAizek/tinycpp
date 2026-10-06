/* ======================================================================
 * nmmintrin.h — SSE4.2 intrinsics for TCC
 * ====================================================================== */

#ifndef _NMMINTRIN_H_INCLUDED
#define _NMMINTRIN_H_INCLUDED

#include "smmintrin.h"

/* 64-bit integer comparison */

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cmpgt_epi64(__m128i __a, __m128i __b) {
    union { __m128i v; long long q[2]; } a, b, r;
    a.v = __a; b.v = __b;
    r.q[0] = (a.q[0] > b.q[0]) ? -1LL : 0LL;
    r.q[1] = (a.q[1] > b.q[1]) ? -1LL : 0LL;
    return r.v;
}

/* CRC32 intrinsics (Castagnoli polynomial 0x1EDC6F41) */

static __inline__ unsigned int __attribute__((__always_inline__))
_mm_crc32_u8(unsigned int __crc, unsigned char __v) {
    unsigned int crc = __crc ^ __v;
    int i;
    for (i = 0; i < 8; i++)
        crc = (crc >> 1) ^ (0x82F63B78 & -(crc & 1));
    return crc;
}

static __inline__ unsigned int __attribute__((__always_inline__))
_mm_crc32_u16(unsigned int __crc, unsigned short __v) {
    __crc = _mm_crc32_u8(__crc, (unsigned char)__v);
    return _mm_crc32_u8(__crc, (unsigned char)(__v >> 8));
}

static __inline__ unsigned int __attribute__((__always_inline__))
_mm_crc32_u32(unsigned int __crc, unsigned int __v) {
    __crc = _mm_crc32_u16(__crc, (unsigned short)__v);
    return _mm_crc32_u16(__crc, (unsigned short)(__v >> 16));
}

#ifdef __x86_64__
static __inline__ unsigned long long __attribute__((__always_inline__))
_mm_crc32_u64(unsigned long long __crc, unsigned long long __v) {
    unsigned int c = _mm_crc32_u32((unsigned int)__crc, (unsigned int)__v);
    return _mm_crc32_u32(c, (unsigned int)(__v >> 32));
}
#endif

/* String comparison flags */

#define _SIDD_UBYTE_OPS                 0x00
#define _SIDD_UWORD_OPS                 0x01
#define _SIDD_SBYTE_OPS                 0x02
#define _SIDD_SWORD_OPS                 0x03

#define _SIDD_CMP_EQUAL_ANY             0x00
#define _SIDD_CMP_RANGES                0x04
#define _SIDD_CMP_EQUAL_EACH            0x08
#define _SIDD_CMP_EQUAL_ORDERED         0x0c

#define _SIDD_POSITIVE_POLARITY         0x00
#define _SIDD_NEGATIVE_POLARITY         0x10
#define _SIDD_MASKED_POSITIVE_POLARITY  0x20
#define _SIDD_MASKED_NEGATIVE_POLARITY  0x30

#define _SIDD_LEAST_SIGNIFICANT         0x00
#define _SIDD_MOST_SIGNIFICANT          0x40

#define _SIDD_BIT_MASK                  0x00
#define _SIDD_UNIT_MASK                 0x40

#endif /* _NMMINTRIN_H_INCLUDED */
