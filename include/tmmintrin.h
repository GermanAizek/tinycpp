/* ======================================================================
 * tmmintrin.h — SSSE3 intrinsics for TCC
 * ====================================================================== */

#ifndef _TMMINTRIN_H_INCLUDED
#define _TMMINTRIN_H_INCLUDED

#include "pmmintrin.h"

/* Horizontal add / subtract */

static __inline__ __m128i __attribute__((__always_inline__))
_mm_hadd_epi16(__m128i __a, __m128i __b) {
    union { __m128i v; short s[8]; } a, b, r;
    a.v = __a; b.v = __b;
    r.s[0] = a.s[0] + a.s[1];
    r.s[1] = a.s[2] + a.s[3];
    r.s[2] = a.s[4] + a.s[5];
    r.s[3] = a.s[6] + a.s[7];
    r.s[4] = b.s[0] + b.s[1];
    r.s[5] = b.s[2] + b.s[3];
    r.s[6] = b.s[4] + b.s[5];
    r.s[7] = b.s[6] + b.s[7];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_hadd_epi32(__m128i __a, __m128i __b) {
    union { __m128i v; int i[4]; } a, b, r;
    a.v = __a; b.v = __b;
    r.i[0] = a.i[0] + a.i[1];
    r.i[1] = a.i[2] + a.i[3];
    r.i[2] = b.i[0] + b.i[1];
    r.i[3] = b.i[2] + b.i[3];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_hadds_epi16(__m128i __a, __m128i __b) {
    union { __m128i v; short s[8]; } a, b, r;
    int k, sum;
    a.v = __a; b.v = __b;
    for (k = 0; k < 4; k++) {
        sum = (int)a.s[2*k] + (int)a.s[2*k + 1];
        r.s[k] = (sum > 32767) ? 32767 : ((sum < -32768) ? -32768 : sum);
    }
    for (k = 0; k < 4; k++) {
        sum = (int)b.s[2*k] + (int)b.s[2*k + 1];
        r.s[4 + k] = (sum > 32767) ? 32767 : ((sum < -32768) ? -32768 : sum);
    }
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_hsub_epi16(__m128i __a, __m128i __b) {
    union { __m128i v; short s[8]; } a, b, r;
    a.v = __a; b.v = __b;
    r.s[0] = a.s[0] - a.s[1];
    r.s[1] = a.s[2] - a.s[3];
    r.s[2] = a.s[4] - a.s[5];
    r.s[3] = a.s[6] - a.s[7];
    r.s[4] = b.s[0] - b.s[1];
    r.s[5] = b.s[2] - b.s[3];
    r.s[6] = b.s[4] - b.s[5];
    r.s[7] = b.s[6] - b.s[7];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_hsub_epi32(__m128i __a, __m128i __b) {
    union { __m128i v; int i[4]; } a, b, r;
    a.v = __a; b.v = __b;
    r.i[0] = a.i[0] - a.i[1];
    r.i[1] = a.i[2] - a.i[3];
    r.i[2] = b.i[0] - b.i[1];
    r.i[3] = b.i[2] - b.i[3];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_hsubs_epi16(__m128i __a, __m128i __b) {
    union { __m128i v; short s[8]; } a, b, r;
    int k, diff;
    a.v = __a; b.v = __b;
    for (k = 0; k < 4; k++) {
        diff = (int)a.s[2*k] - (int)a.s[2*k + 1];
        r.s[k] = (diff > 32767) ? 32767 : ((diff < -32768) ? -32768 : diff);
    }
    for (k = 0; k < 4; k++) {
        diff = (int)b.s[2*k] - (int)b.s[2*k + 1];
        r.s[4 + k] = (diff > 32767) ? 32767 : ((diff < -32768) ? -32768 : diff);
    }
    return r.v;
}

/* Absolute value */

static __inline__ __m128i __attribute__((__always_inline__))
_mm_abs_epi8(__m128i __a) {
    union { __m128i v; signed char b[16]; } a, r;
    int i;
    a.v = __a;
    for (i = 0; i < 16; i++)
        r.b[i] = (a.b[i] < 0) ? -a.b[i] : a.b[i];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_abs_epi16(__m128i __a) {
    union { __m128i v; short s[8]; } a, r;
    int i;
    a.v = __a;
    for (i = 0; i < 8; i++)
        r.s[i] = (a.s[i] < 0) ? -a.s[i] : a.s[i];
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_abs_epi32(__m128i __a) {
    union { __m128i v; int i[4]; } a, r;
    int k;
    a.v = __a;
    for (k = 0; k < 4; k++)
        r.i[k] = (a.i[k] < 0) ? -a.i[k] : a.i[k];
    return r.v;
}

/* Sign */

static __inline__ __m128i __attribute__((__always_inline__))
_mm_sign_epi8(__m128i __a, __m128i __b) {
    union { __m128i v; signed char b[16]; } a, b, r;
    int i;
    a.v = __a; b.v = __b;
    for (i = 0; i < 16; i++)
        r.b[i] = (b.b[i] < 0) ? -a.b[i] : ((b.b[i] > 0) ? a.b[i] : 0);
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_sign_epi16(__m128i __a, __m128i __b) {
    union { __m128i v; short s[8]; } a, b, r;
    int i;
    a.v = __a; b.v = __b;
    for (i = 0; i < 8; i++)
        r.s[i] = (b.s[i] < 0) ? -a.s[i] : ((b.s[i] > 0) ? a.s[i] : 0);
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_sign_epi32(__m128i __a, __m128i __b) {
    union { __m128i v; int i[4]; } a, b, r;
    int k;
    a.v = __a; b.v = __b;
    for (k = 0; k < 4; k++)
        r.i[k] = (b.i[k] < 0) ? -a.i[k] : ((b.i[k] > 0) ? a.i[k] : 0);
    return r.v;
}

/* Byte shuffle */

static __inline__ __m128i __attribute__((__always_inline__))
_mm_shuffle_epi8(__m128i __a, __m128i __b) {
    union { __m128i v; unsigned char b[16]; signed char sb[16]; } a, b, r;
    int i;
    a.v = __a; b.v = __b;
    for (i = 0; i < 16; i++) {
        if (b.sb[i] < 0)
            r.b[i] = 0;
        else
            r.b[i] = a.b[b.b[i] & 0x0F];
    }
    return r.v;
}

/* Multiply and add */

static __inline__ __m128i __attribute__((__always_inline__))
_mm_maddubs_epi16(__m128i __a, __m128i __b) {
    union { __m128i v; unsigned char ub[16]; signed char sb[16]; short s[8]; } a, b, r;
    int i, p;
    a.v = __a; b.v = __b;
    for (i = 0; i < 8; i++) {
        p = ((int)a.ub[2*i] * (int)b.sb[2*i]) +
            ((int)a.ub[2*i + 1] * (int)b.sb[2*i + 1]);
        r.s[i] = (p > 32767) ? 32767 : ((p < -32768) ? -32768 : p);
    }
    return r.v;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_mulhrs_epi16(__m128i __a, __m128i __b) {
    union { __m128i v; short s[8]; } a, b, r;
    int i, p;
    a.v = __a; b.v = __b;
    for (i = 0; i < 8; i++) {
        p = (((int)a.s[i] * (int)b.s[i]) + 0x4000) >> 15;
        r.s[i] = (short)p;
    }
    return r.v;
}

/* Align right */

#define _mm_alignr_epi8(__a, __b, __count) __extension__ ({ \
    union { __m128i v; unsigned char b[16]; } __u_a, __u_b, __u_r; \
    int __k; \
    __u_a.v = (__a); __u_b.v = (__b); \
    for (__k = 0; __k < 16; __k++) { \
        int __idx = (__count) + __k; \
        if (__idx < 16) __u_r.b[__k] = __u_b.b[__idx]; \
        else if (__idx < 32) __u_r.b[__k] = __u_a.b[__idx - 16]; \
        else __u_r.b[__k] = 0; \
    } \
    __u_r.v; \
})

#endif /* _TMMINTRIN_H_INCLUDED */
