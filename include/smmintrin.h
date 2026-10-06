/* ======================================================================
 * smmintrin.h — SSE4.1 intrinsics for TCC
 * ====================================================================== */

#ifndef _SMMINTRIN_H_INCLUDED
#define _SMMINTRIN_H_INCLUDED

#include "tmmintrin.h"

/* Rounding modes */
#define _MM_FROUND_TO_NEAREST_INT 0x00
#define _MM_FROUND_TO_NEG_INF     0x01
#define _MM_FROUND_TO_POS_INF     0x02
#define _MM_FROUND_TO_ZERO        0x03
#define _MM_FROUND_CUR_DIRECTION  0x04

#define _MM_FROUND_RAISE_EXC      0x00
#define _MM_FROUND_NO_EXC         0x08

#define _MM_FROUND_NINT  (_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_FLOOR (_MM_FROUND_TO_NEG_INF | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_CEIL  (_MM_FROUND_TO_POS_INF | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_TRUNC (_MM_FROUND_TO_ZERO | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_RINT  (_MM_FROUND_CUR_DIRECTION | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_NEARBYINT (_MM_FROUND_CUR_DIRECTION | _MM_FROUND_NO_EXC)

/* Blends */

#define _mm_blend_pd(__a, __b, __imm) __extension__ ({ \
    union { __m128d v; double d[2]; } __u_a, __u_b, __u_r; \
    __u_a.v = (__a); __u_b.v = (__b); \
    __u_r.d[0] = ((__imm) & 1) ? __u_b.d[0] : __u_a.d[0]; \
    __u_r.d[1] = ((__imm) & 2) ? __u_b.d[1] : __u_a.d[1]; \
    __u_r.v; \
})

#define _mm_blend_ps(__a, __b, __imm) __extension__ ({ \
    union { __m128 v; float f[4]; } __u_a, __u_b, __u_r; \
    __u_a.v = (__a); __u_b.v = (__b); \
    __u_r.f[0] = ((__imm) & 1) ? __u_b.f[0] : __u_a.f[0]; \
    __u_r.f[1] = ((__imm) & 2) ? __u_b.f[1] : __u_a.f[1]; \
    __u_r.f[2] = ((__imm) & 4) ? __u_b.f[2] : __u_a.f[2]; \
    __u_r.f[3] = ((__imm) & 8) ? __u_b.f[3] : __u_a.f[3]; \
    __u_r.v; \
})

#define _mm_blend_epi16(__a, __b, __imm) __extension__ ({ \
    union { __m128i v; short s[8]; } __u_a, __u_b, __u_r; \
    int __k; \
    __u_a.v = (__a); __u_b.v = (__b); \
    for (__k = 0; __k < 8; __k++) \
        __u_r.s[__k] = ((__imm) & (1 << __k)) ? __u_b.s[__k] : __u_a.s[__k]; \
    __u_r.v; \
})

static __inline__ __m128d __attribute__((__always_inline__))
_mm_blendv_pd(__m128d __a, __m128d __b, __m128d __mask) {
    union { __m128d v; long long q[2]; double d[2]; } a, b, m, r;
    a.v = __a; b.v = __b; m.v = __mask;
    r.d[0] = (m.q[0] < 0) ? b.d[0] : a.d[0];
    r.d[1] = (m.q[1] < 0) ? b.d[1] : a.d[1];
    return r.v;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_blendv_ps(__m128 __a, __m128 __b, __m128 __mask) {
    union { __m128 v; int i[4]; float f[4]; } a, b, m, r;
    int k;
    a.v = __a; b.v = __b; m.v = __mask;
    for (k = 0; k < 4; k++)
        r.f[k] = (m.i[k] < 0) ? b.f[k] : a.f[k];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_blendv_epi8(__m128i __a, __m128i __b, __m128i __mask) {
    union { __m128i v; signed char b[16]; } a, b, m, r;
    int k;
    a.v = __a; b.v = __b; m.v = __mask;
    for (k = 0; k < 16; k++)
        r.b[k] = (m.b[k] < 0) ? b.b[k] : a.b[k];
    return r.v;
}

/* Integer Min / Max */

static __inline__ __m128i __attribute__((__always_inline__))
_mm_min_epi8(__m128i __a, __m128i __b) {
    union { __m128i v; signed char b[16]; } a, b, r;
    int k;
    a.v = __a; b.v = __b;
    for (k = 0; k < 16; k++)
        r.b[k] = (a.b[k] < b.b[k]) ? a.b[k] : b.b[k];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_max_epi8(__m128i __a, __m128i __b) {
    union { __m128i v; signed char b[16]; } a, b, r;
    int k;
    a.v = __a; b.v = __b;
    for (k = 0; k < 16; k++)
        r.b[k] = (a.b[k] > b.b[k]) ? a.b[k] : b.b[k];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_min_epu16(__m128i __a, __m128i __b) {
    union { __m128i v; unsigned short s[8]; } a, b, r;
    int k;
    a.v = __a; b.v = __b;
    for (k = 0; k < 8; k++)
        r.s[k] = (a.s[k] < b.s[k]) ? a.s[k] : b.s[k];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_max_epu16(__m128i __a, __m128i __b) {
    union { __m128i v; unsigned short s[8]; } a, b, r;
    int k;
    a.v = __a; b.v = __b;
    for (k = 0; k < 8; k++)
        r.s[k] = (a.s[k] > b.s[k]) ? a.s[k] : b.s[k];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_min_epi32(__m128i __a, __m128i __b) {
    union { __m128i v; int i[4]; } a, b, r;
    int k;
    a.v = __a; b.v = __b;
    for (k = 0; k < 4; k++)
        r.i[k] = (a.i[k] < b.i[k]) ? a.i[k] : b.i[k];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_max_epi32(__m128i __a, __m128i __b) {
    union { __m128i v; int i[4]; } a, b, r;
    int k;
    a.v = __a; b.v = __b;
    for (k = 0; k < 4; k++)
        r.i[k] = (a.i[k] > b.i[k]) ? a.i[k] : b.i[k];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_min_epu32(__m128i __a, __m128i __b) {
    union { __m128i v; unsigned int i[4]; } a, b, r;
    int k;
    a.v = __a; b.v = __b;
    for (k = 0; k < 4; k++)
        r.i[k] = (a.i[k] < b.i[k]) ? a.i[k] : b.i[k];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_max_epu32(__m128i __a, __m128i __b) {
    union { __m128i v; unsigned int i[4]; } a, b, r;
    int k;
    a.v = __a; b.v = __b;
    for (k = 0; k < 4; k++)
        r.i[k] = (a.i[k] > b.i[k]) ? a.i[k] : b.i[k];
    return r.v;
}

/* Integer Multiplication */

static __inline__ __m128i __attribute__((__always_inline__))
_mm_mullo_epi32(__m128i __a, __m128i __b) {
    union { __m128i v; int i[4]; } a, b, r;
    int k;
    a.v = __a; b.v = __b;
    for (k = 0; k < 4; k++)
        r.i[k] = a.i[k] * b.i[k];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_mul_epi32(__m128i __a, __m128i __b) {
    union { __m128i v; int i[4]; long long q[2]; } a, b, r;
    a.v = __a; b.v = __b;
    r.q[0] = (long long)a.i[0] * (long long)b.i[0];
    r.q[1] = (long long)a.i[2] * (long long)b.i[2];
    return r.v;
}

/* 64-bit comparison */

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cmpeq_epi64(__m128i __a, __m128i __b) {
    union { __m128i v; long long q[2]; } a, b, r;
    a.v = __a; b.v = __b;
    r.q[0] = (a.q[0] == b.q[0]) ? -1LL : 0LL;
    r.q[1] = (a.q[1] == b.q[1]) ? -1LL : 0LL;
    return r.v;
}

/* Sign / Zero extensions */

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cvtepi8_epi16(__m128i __a) {
    union { __m128i v; signed char b[16]; short s[8]; } a, r;
    int k;
    a.v = __a;
    for (k = 0; k < 8; k++) r.s[k] = a.b[k];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cvtepi8_epi32(__m128i __a) {
    union { __m128i v; signed char b[16]; int i[4]; } a, r;
    int k;
    a.v = __a;
    for (k = 0; k < 4; k++) r.i[k] = a.b[k];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cvtepi8_epi64(__m128i __a) {
    union { __m128i v; signed char b[16]; long long q[2]; } a, r;
    a.v = __a;
    r.q[0] = a.b[0]; r.q[1] = a.b[1];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cvtepi16_epi32(__m128i __a) {
    union { __m128i v; short s[8]; int i[4]; } a, r;
    int k;
    a.v = __a;
    for (k = 0; k < 4; k++) r.i[k] = a.s[k];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cvtepi16_epi64(__m128i __a) {
    union { __m128i v; short s[8]; long long q[2]; } a, r;
    a.v = __a;
    r.q[0] = a.s[0]; r.q[1] = a.s[1];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cvtepi32_epi64(__m128i __a) {
    union { __m128i v; int i[4]; long long q[2]; } a, r;
    a.v = __a;
    r.q[0] = a.i[0]; r.q[1] = a.i[1];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cvtepu8_epi16(__m128i __a) {
    union { __m128i v; unsigned char b[16]; short s[8]; } a, r;
    int k;
    a.v = __a;
    for (k = 0; k < 8; k++) r.s[k] = a.b[k];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cvtepu8_epi32(__m128i __a) {
    union { __m128i v; unsigned char b[16]; int i[4]; } a, r;
    int k;
    a.v = __a;
    for (k = 0; k < 4; k++) r.i[k] = a.b[k];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cvtepu8_epi64(__m128i __a) {
    union { __m128i v; unsigned char b[16]; long long q[2]; } a, r;
    a.v = __a;
    r.q[0] = a.b[0]; r.q[1] = a.b[1];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cvtepu16_epi32(__m128i __a) {
    union { __m128i v; unsigned short s[8]; int i[4]; } a, r;
    int k;
    a.v = __a;
    for (k = 0; k < 4; k++) r.i[k] = a.s[k];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cvtepu16_epi64(__m128i __a) {
    union { __m128i v; unsigned short s[8]; long long q[2]; } a, r;
    a.v = __a;
    r.q[0] = a.s[0]; r.q[1] = a.s[1];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cvtepu32_epi64(__m128i __a) {
    union { __m128i v; unsigned int i[4]; long long q[2]; } a, r;
    a.v = __a;
    r.q[0] = a.i[0]; r.q[1] = a.i[1];
    return r.v;
}

/* Insert / Extract */

#define _mm_extract_epi8(__a, __imm) __extension__ ({ \
    union { __m128i v; unsigned char b[16]; } __u; \
    __u.v = (__a); \
    (int)__u.b[(__imm) & 15]; \
})

#define _mm_extract_epi32(__a, __imm) __extension__ ({ \
    union { __m128i v; int i[4]; } __u; \
    __u.v = (__a); \
    __u.i[(__imm) & 3]; \
})

#define _mm_extract_epi64(__a, __imm) __extension__ ({ \
    union { __m128i v; long long q[2]; } __u; \
    __u.v = (__a); \
    __u.q[(__imm) & 1]; \
})

#define _mm_insert_epi8(__a, __i, __imm) __extension__ ({ \
    union { __m128i v; unsigned char b[16]; } __u; \
    __u.v = (__a); \
    __u.b[(__imm) & 15] = (unsigned char)(__i); \
    __u.v; \
})

#define _mm_insert_epi32(__a, __i, __imm) __extension__ ({ \
    union { __m128i v; int i[4]; } __u; \
    __u.v = (__a); \
    __u.i[(__imm) & 3] = (__i); \
    __u.v; \
})

#define _mm_insert_epi64(__a, __i, __imm) __extension__ ({ \
    union { __m128i v; long long q[2]; } __u; \
    __u.v = (__a); \
    __u.q[(__imm) & 1] = (__i); \
    __u.v; \
})

/* Test instructions */

static __inline__ int __attribute__((__always_inline__))
_mm_testz_si128(__m128i __a, __m128i __b) {
    union { __m128i v; long long q[2]; } a, b;
    a.v = __a; b.v = __b;
    return ((a.q[0] & b.q[0]) == 0) && ((a.q[1] & b.q[1]) == 0);
}

static __inline__ int __attribute__((__always_inline__))
_mm_testc_si128(__m128i __a, __m128i __b) {
    union { __m128i v; long long q[2]; } a, b;
    a.v = __a; b.v = __b;
    return (((~a.q[0]) & b.q[0]) == 0) && (((~a.q[1]) & b.q[1]) == 0);
}

static __inline__ int __attribute__((__always_inline__))
_mm_testnzc_si128(__m128i __a, __m128i __b) {
    return !_mm_testz_si128(__a, __b) && !_mm_testc_si128(__a, __b);
}

#endif /* _SMMINTRIN_H_INCLUDED */
