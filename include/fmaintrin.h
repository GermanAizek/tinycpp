/* ======================================================================
 * fmaintrin.h — FMA3 intrinsics for TCC
 * ====================================================================== */

#ifndef _FMAINTRIN_H_INCLUDED
#define _FMAINTRIN_H_INCLUDED

#include "avxintrin.h"

/* 128-bit FMA: computes a * b + c */

static __inline__ __m128 __attribute__((__always_inline__))
_mm_fmadd_ps(__m128 __a, __m128 __b, __m128 __c) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.f[__k] = (__a.f[__k] * __b.f[__k]) + __c.f[__k];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_fmadd_ss(__m128 __a, __m128 __b, __m128 __c) {
    __m128 __r = __a;
    __r.f[0] = (__a.f[0] * __b.f[0]) + __c.f[0];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_fmadd_pd(__m128d __a, __m128d __b, __m128d __c) {
    __m128d __r; int __k;
    for (__k = 0; __k < 2; __k++) __r.d[__k] = (__a.d[__k] * __b.d[__k]) + __c.d[__k];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_fmadd_sd(__m128d __a, __m128d __b, __m128d __c) {
    __m128d __r = __a;
    __r.d[0] = (__a.d[0] * __b.d[0]) + __c.d[0];
    return __r;
}

/* 128-bit FMSUB: computes a * b - c */

static __inline__ __m128 __attribute__((__always_inline__))
_mm_fmsub_ps(__m128 __a, __m128 __b, __m128 __c) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.f[__k] = (__a.f[__k] * __b.f[__k]) - __c.f[__k];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_fmsub_ss(__m128 __a, __m128 __b, __m128 __c) {
    __m128 __r = __a;
    __r.f[0] = (__a.f[0] * __b.f[0]) - __c.f[0];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_fmsub_pd(__m128d __a, __m128d __b, __m128d __c) {
    __m128d __r; int __k;
    for (__k = 0; __k < 2; __k++) __r.d[__k] = (__a.d[__k] * __b.d[__k]) - __c.d[__k];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_fmsub_sd(__m128d __a, __m128d __b, __m128d __c) {
    __m128d __r = __a;
    __r.d[0] = (__a.d[0] * __b.d[0]) - __c.d[0];
    return __r;
}

/* 128-bit FNMADD: computes -(a * b) + c */

static __inline__ __m128 __attribute__((__always_inline__))
_mm_fnmadd_ps(__m128 __a, __m128 __b, __m128 __c) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.f[__k] = -(__a.f[__k] * __b.f[__k]) + __c.f[__k];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_fnmadd_ss(__m128 __a, __m128 __b, __m128 __c) {
    __m128 __r = __a;
    __r.f[0] = -(__a.f[0] * __b.f[0]) + __c.f[0];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_fnmadd_pd(__m128d __a, __m128d __b, __m128d __c) {
    __m128d __r; int __k;
    for (__k = 0; __k < 2; __k++) __r.d[__k] = -(__a.d[__k] * __b.d[__k]) + __c.d[__k];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_fnmadd_sd(__m128d __a, __m128d __b, __m128d __c) {
    __m128d __r = __a;
    __r.d[0] = -(__a.d[0] * __b.d[0]) + __c.d[0];
    return __r;
}

/* 128-bit FNMSUB: computes -(a * b) - c */

static __inline__ __m128 __attribute__((__always_inline__))
_mm_fnmsub_ps(__m128 __a, __m128 __b, __m128 __c) {
    __m128 __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.f[__k] = -(__a.f[__k] * __b.f[__k]) - __c.f[__k];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_fnmsub_ss(__m128 __a, __m128 __b, __m128 __c) {
    __m128 __r = __a;
    __r.f[0] = -(__a.f[0] * __b.f[0]) - __c.f[0];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_fnmsub_pd(__m128d __a, __m128d __b, __m128d __c) {
    __m128d __r; int __k;
    for (__k = 0; __k < 2; __k++) __r.d[__k] = -(__a.d[__k] * __b.d[__k]) - __c.d[__k];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_fnmsub_sd(__m128d __a, __m128d __b, __m128d __c) {
    __m128d __r = __a;
    __r.d[0] = -(__a.d[0] * __b.d[0]) - __c.d[0];
    return __r;
}

/* 256-bit FMA */

#define _mm256_fmadd_ps(__a, __b, __c) __extension__ ({ \
    __m256 __u_a = (__a), __u_b = (__b), __u_c = (__c), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.f[__k] = (__u_a.f[__k] * __u_b.f[__k]) + __u_c.f[__k]; \
    __r; \
})

#define _mm256_fmadd_pd(__a, __b, __c) __extension__ ({ \
    __m256d __u_a = (__a), __u_b = (__b), __u_c = (__c), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.d[__k] = (__u_a.d[__k] * __u_b.d[__k]) + __u_c.d[__k]; \
    __r; \
})

#define _mm256_fmsub_ps(__a, __b, __c) __extension__ ({ \
    __m256 __u_a = (__a), __u_b = (__b), __u_c = (__c), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.f[__k] = (__u_a.f[__k] * __u_b.f[__k]) - __u_c.f[__k]; \
    __r; \
})

#define _mm256_fmsub_pd(__a, __b, __c) __extension__ ({ \
    __m256d __u_a = (__a), __u_b = (__b), __u_c = (__c), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.d[__k] = (__u_a.d[__k] * __u_b.d[__k]) - __u_c.d[__k]; \
    __r; \
})

#define _mm256_fnmadd_ps(__a, __b, __c) __extension__ ({ \
    __m256 __u_a = (__a), __u_b = (__b), __u_c = (__c), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.f[__k] = -(__u_a.f[__k] * __u_b.f[__k]) + __u_c.f[__k]; \
    __r; \
})

#define _mm256_fnmadd_pd(__a, __b, __c) __extension__ ({ \
    __m256d __u_a = (__a), __u_b = (__b), __u_c = (__c), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.d[__k] = -(__u_a.d[__k] * __u_b.d[__k]) + __u_c.d[__k]; \
    __r; \
})

#define _mm256_fnmsub_ps(__a, __b, __c) __extension__ ({ \
    __m256 __u_a = (__a), __u_b = (__b), __u_c = (__c), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.f[__k] = -(__u_a.f[__k] * __u_b.f[__k]) - __u_c.f[__k]; \
    __r; \
})

#define _mm256_fnmsub_pd(__a, __b, __c) __extension__ ({ \
    __m256d __u_a = (__a), __u_b = (__b), __u_c = (__c), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.d[__k] = -(__u_a.d[__k] * __u_b.d[__k]) - __u_c.d[__k]; \
    __r; \
})

#endif /* _FMAINTRIN_H_INCLUDED */
