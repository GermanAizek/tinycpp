/* ======================================================================
 * xmmintrin.h — Streaming SIMD Extensions (SSE) intrinsics for TCC
 * ====================================================================== */

#ifndef _XMMINTRIN_H_INCLUDED
#define _XMMINTRIN_H_INCLUDED

#include <stddef.h>
#include <string.h>
#include <math.h>

#define _MM_SHUFFLE(fp3,fp2,fp1,fp0) (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | (fp0))

typedef union {
    float              f[4];
    double             d[2];
    signed char        b[16];
    short              s[8];
    int                i[4];
    long long          q[2];
    unsigned char      ub[16];
    unsigned short     us[8];
    unsigned int       ui[4];
    unsigned long long uq[2];
    float              m128_f32[4];
    double             m128_f64[2];
    signed char        m128_i8[16];
    short              m128_i16[8];
    int                m128_i32[4];
    long long          m128_i64[2];
    unsigned char      m128_u8[16];
    unsigned short     m128_u16[8];
    unsigned int       m128_u32[4];
    unsigned long long m128_u64[2];
} __attribute__((__aligned__(16))) __m128;

/* Arithmetic */

static __inline__ __m128 __attribute__((__always_inline__))
_mm_add_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.f[__k] = __a.f[__k] + __b.f[__k];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_add_ss(__m128 __a, __m128 __b) {
    __m128 __r = __a;
    __r.f[0] = __a.f[0] + __b.f[0];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_sub_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.f[__k] = __a.f[__k] - __b.f[__k];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_sub_ss(__m128 __a, __m128 __b) {
    __m128 __r = __a;
    __r.f[0] = __a.f[0] - __b.f[0];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_mul_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.f[__k] = __a.f[__k] * __b.f[__k];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_mul_ss(__m128 __a, __m128 __b) {
    __m128 __r = __a;
    __r.f[0] = __a.f[0] * __b.f[0];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_div_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.f[__k] = __a.f[__k] / __b.f[__k];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_div_ss(__m128 __a, __m128 __b) {
    __m128 __r = __a;
    __r.f[0] = __a.f[0] / __b.f[0];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_sqrt_ps(__m128 __a) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.f[__k] = sqrtf(__a.f[__k]);
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_sqrt_ss(__m128 __a) {
    __m128 __r = __a;
    __r.f[0] = sqrtf(__a.f[0]);
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_rcp_ps(__m128 __a) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.f[__k] = 1.0f / __a.f[__k];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_rcp_ss(__m128 __a) {
    __m128 __r = __a;
    __r.f[0] = 1.0f / __a.f[0];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_rsqrt_ps(__m128 __a) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.f[__k] = 1.0f / sqrtf(__a.f[__k]);
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_rsqrt_ss(__m128 __a) {
    __m128 __r = __a;
    __r.f[0] = 1.0f / sqrtf(__a.f[0]);
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_min_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++)
        __r.f[__k] = (__a.f[__k] < __b.f[__k]) ? __a.f[__k] : __b.f[__k];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_min_ss(__m128 __a, __m128 __b) {
    __m128 __r = __a;
    __r.f[0] = (__a.f[0] < __b.f[0]) ? __a.f[0] : __b.f[0];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_max_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++)
        __r.f[__k] = (__a.f[__k] > __b.f[__k]) ? __a.f[__k] : __b.f[__k];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_max_ss(__m128 __a, __m128 __b) {
    __m128 __r = __a;
    __r.f[0] = (__a.f[0] > __b.f[0]) ? __a.f[0] : __b.f[0];
    return __r;
}

/* Logical operations */

static __inline__ __m128 __attribute__((__always_inline__))
_mm_and_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.i[__k] = __a.i[__k] & __b.i[__k];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_andnot_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.i[__k] = (~__a.i[__k]) & __b.i[__k];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_or_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.i[__k] = __a.i[__k] | __b.i[__k];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_xor_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.i[__k] = __a.i[__k] ^ __b.i[__k];
    return __r;
}

/* Comparisons */

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpeq_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.i[__k] = (__a.f[__k] == __b.f[__k]) ? -1 : 0;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmplt_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.i[__k] = (__a.f[__k] < __b.f[__k]) ? -1 : 0;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmple_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.i[__k] = (__a.f[__k] <= __b.f[__k]) ? -1 : 0;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpgt_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.i[__k] = (__a.f[__k] > __b.f[__k]) ? -1 : 0;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpge_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.i[__k] = (__a.f[__k] >= __b.f[__k]) ? -1 : 0;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpneq_ps(__m128 __a, __m128 __b) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.i[__k] = (__a.f[__k] != __b.f[__k]) ? -1 : 0;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpeq_ss(__m128 __a, __m128 __b) {
    __m128 __r = __a;
    __r.i[0] = (__a.f[0] == __b.f[0]) ? -1 : 0;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmplt_ss(__m128 __a, __m128 __b) {
    __m128 __r = __a;
    __r.i[0] = (__a.f[0] < __b.f[0]) ? -1 : 0;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmple_ss(__m128 __a, __m128 __b) {
    __m128 __r = __a;
    __r.i[0] = (__a.f[0] <= __b.f[0]) ? -1 : 0;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpgt_ss(__m128 __a, __m128 __b) {
    __m128 __r = __a;
    __r.i[0] = (__a.f[0] > __b.f[0]) ? -1 : 0;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpge_ss(__m128 __a, __m128 __b) {
    __m128 __r = __a;
    __r.i[0] = (__a.f[0] >= __b.f[0]) ? -1 : 0;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpneq_ss(__m128 __a, __m128 __b) {
    __m128 __r = __a;
    __r.i[0] = (__a.f[0] != __b.f[0]) ? -1 : 0;
    return __r;
}

/* Set / Load / Store */

static __inline__ __m128 __attribute__((__always_inline__))
_mm_setzero_ps(void) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.f[__k] = 0.0f;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_set_ss(float __w) {
    __m128 __r;
    __r.f[0] = __w; __r.f[1] = 0.0f; __r.f[2] = 0.0f; __r.f[3] = 0.0f;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_set1_ps(float __w) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.f[__k] = __w;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_set_ps(float __e3, float __e2, float __e1, float __e0) {
    __m128 __r;
    __r.f[0] = __e0; __r.f[1] = __e1; __r.f[2] = __e2; __r.f[3] = __e3;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_setr_ps(float __e0, float __e1, float __e2, float __e3) {
    return _mm_set_ps(__e3, __e2, __e1, __e0);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_load_ps(float const *__p) {
    __m128 __r;
    memcpy(&__r, __p, sizeof(__m128));
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_loadu_ps(float const *__p) {
    __m128 __r;
    memcpy(&__r, __p, sizeof(__m128));
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_load_ss(float const *__p) {
    __m128 __r = _mm_setzero_ps();
    __r.f[0] = *__p;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_load1_ps(float const *__p) {
    return _mm_set1_ps(*__p);
}

static __inline__ void __attribute__((__always_inline__))
_mm_store_ps(float *__p, __m128 __a) {
    memcpy(__p, &__a, sizeof(__m128));
}

static __inline__ void __attribute__((__always_inline__))
_mm_storeu_ps(float *__p, __m128 __a) {
    memcpy(__p, &__a, sizeof(__m128));
}

static __inline__ void __attribute__((__always_inline__))
_mm_store_ss(float *__p, __m128 __a) {
    *__p = __a.f[0];
}

static __inline__ void __attribute__((__always_inline__))
_mm_store1_ps(float *__p, __m128 __a) {
    __p[0] = __a.f[0]; __p[1] = __a.f[0];
    __p[2] = __a.f[0]; __p[3] = __a.f[0];
}

/* Moves and shuffles */

static __inline__ __m128 __attribute__((__always_inline__))
_mm_move_ss(__m128 __a, __m128 __b) {
    __m128 __r = __a;
    __r.f[0] = __b.f[0];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_movehl_ps(__m128 __a, __m128 __b) {
    __m128 __r;
    __r.f[0] = __b.f[2]; __r.f[1] = __b.f[3];
    __r.f[2] = __a.f[2]; __r.f[3] = __a.f[3];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_movelh_ps(__m128 __a, __m128 __b) {
    __m128 __r;
    __r.f[0] = __a.f[0]; __r.f[1] = __a.f[1];
    __r.f[2] = __b.f[0]; __r.f[3] = __b.f[1];
    return __r;
}

static __inline__ int __attribute__((__always_inline__))
_mm_movemask_ps(__m128 __a) {
    return ((__a.i[0] < 0) ? 1 : 0) |
           ((__a.i[1] < 0) ? 2 : 0) |
           ((__a.i[2] < 0) ? 4 : 0) |
           ((__a.i[3] < 0) ? 8 : 0);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_unpackhi_ps(__m128 __a, __m128 __b) {
    __m128 __r;
    __r.f[0] = __a.f[2]; __r.f[1] = __b.f[2];
    __r.f[2] = __a.f[3]; __r.f[3] = __b.f[3];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_unpacklo_ps(__m128 __a, __m128 __b) {
    __m128 __r;
    __r.f[0] = __a.f[0]; __r.f[1] = __b.f[0];
    __r.f[2] = __a.f[1]; __r.f[3] = __b.f[1];
    return __r;
}

#define _mm_shuffle_ps(__a, __b, __mask) __extension__ ({ \
    __m128 __u_a = (__a), __u_b = (__b), __u_r; \
    __u_r.f[0] = __u_a.f[((__mask)     ) & 3]; \
    __u_r.f[1] = __u_a.f[((__mask) >> 2) & 3]; \
    __u_r.f[2] = __u_b.f[((__mask) >> 4) & 3]; \
    __u_r.f[3] = __u_b.f[((__mask) >> 6) & 3]; \
    __u_r; \
})

/* Conversions */

static __inline__ int __attribute__((__always_inline__))
_mm_cvtss_si32(__m128 __a) {
    return (int)lrintf(__a.f[0]);
}

static __inline__ long long __attribute__((__always_inline__))
_mm_cvtss_si64(__m128 __a) {
    return (long long)llrintf(__a.f[0]);
}

static __inline__ int __attribute__((__always_inline__))
_mm_cvttss_si32(__m128 __a) {
    return (int)__a.f[0];
}

static __inline__ long long __attribute__((__always_inline__))
_mm_cvttss_si64(__m128 __a) {
    return (long long)__a.f[0];
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cvtsi32_ss(__m128 __a, int __b) {
    __m128 __r = __a;
    __r.f[0] = (float)__b;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cvtsi64_ss(__m128 __a, long long __b) {
    __m128 __r = __a;
    __r.f[0] = (float)__b;
    return __r;
}

/* Fences */

static __inline__ void __attribute__((__always_inline__))
_mm_pause(void) {
    __asm__ __volatile__("pause");
}

static __inline__ void __attribute__((__always_inline__))
_mm_sfence(void) {
    __asm__ __volatile__("sfence" ::: "memory");
}

#endif /* _XMMINTRIN_H_INCLUDED */
