/* ======================================================================
 * avxintrin.h — Advanced Vector Extensions (AVX) intrinsics for TCC
 * ====================================================================== */

#ifndef _AVXINTRIN_H_INCLUDED
#define _AVXINTRIN_H_INCLUDED

#include "wmmintrin.h"

typedef union {
    float              f[8];
    double             d[4];
    signed char        b[32];
    short              s[16];
    int                i[8];
    long long          q[4];
    unsigned char      ub[32];
    unsigned short     us[16];
    unsigned int       ui[8];
    unsigned long long uq[4];
    float              m256_f32[8];
    double             m256_f64[4];
    signed char        m256_i8[32];
    short              m256_i16[16];
    int                m256_i32[8];
    long long          m256_i64[4];
    unsigned char      m256_u8[32];
    unsigned short     m256_u16[16];
    unsigned int       m256_u32[8];
    unsigned long long m256_u64[4];
} __attribute__((__aligned__(32))) __m256;

typedef __m256 __m256d;
typedef __m256 __m256i;

/* State management */

static __inline__ void __attribute__((__always_inline__))
_mm256_zeroupper(void) {
    __asm__ __volatile__("vzeroupper");
}

static __inline__ void __attribute__((__always_inline__))
_mm256_zeroall(void) {
    __asm__ __volatile__("vzeroall");
}

/* Set zero */

#define _mm256_setzero_ps() __extension__ ({ \
    __m256 __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.f[__k] = 0.0f; \
    __r; \
})

#define _mm256_setzero_pd() __extension__ ({ \
    __m256d __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.d[__k] = 0.0; \
    __r; \
})

#define _mm256_setzero_si256() __extension__ ({ \
    __m256i __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.q[__k] = 0LL; \
    __r; \
})

/* Single-precision floating point arithmetic */

#define _mm256_add_ps(__a, __b) __extension__ ({ \
    __m256 __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.f[__k] = __u_a.f[__k] + __u_b.f[__k]; \
    __r; \
})

#define _mm256_sub_ps(__a, __b) __extension__ ({ \
    __m256 __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.f[__k] = __u_a.f[__k] - __u_b.f[__k]; \
    __r; \
})

#define _mm256_mul_ps(__a, __b) __extension__ ({ \
    __m256 __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.f[__k] = __u_a.f[__k] * __u_b.f[__k]; \
    __r; \
})

#define _mm256_div_ps(__a, __b) __extension__ ({ \
    __m256 __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.f[__k] = __u_a.f[__k] / __u_b.f[__k]; \
    __r; \
})

#define _mm256_sqrt_ps(__a) __extension__ ({ \
    __m256 __u_a = (__a), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.f[__k] = sqrtf(__u_a.f[__k]); \
    __r; \
})

#define _mm256_min_ps(__a, __b) __extension__ ({ \
    __m256 __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) \
        __r.f[__k] = (__u_a.f[__k] < __u_b.f[__k]) ? __u_a.f[__k] : __u_b.f[__k]; \
    __r; \
})

#define _mm256_max_ps(__a, __b) __extension__ ({ \
    __m256 __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) \
        __r.f[__k] = (__u_a.f[__k] > __u_b.f[__k]) ? __u_a.f[__k] : __u_b.f[__k]; \
    __r; \
})

/* Double-precision floating point arithmetic */

#define _mm256_add_pd(__a, __b) __extension__ ({ \
    __m256d __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.d[__k] = __u_a.d[__k] + __u_b.d[__k]; \
    __r; \
})

#define _mm256_sub_pd(__a, __b) __extension__ ({ \
    __m256d __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.d[__k] = __u_a.d[__k] - __u_b.d[__k]; \
    __r; \
})

#define _mm256_mul_pd(__a, __b) __extension__ ({ \
    __m256d __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.d[__k] = __u_a.d[__k] * __u_b.d[__k]; \
    __r; \
})

#define _mm256_div_pd(__a, __b) __extension__ ({ \
    __m256d __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.d[__k] = __u_a.d[__k] / __u_b.d[__k]; \
    __r; \
})

#define _mm256_sqrt_pd(__a) __extension__ ({ \
    __m256d __u_a = (__a), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.d[__k] = sqrt(__u_a.d[__k]); \
    __r; \
})

#define _mm256_min_pd(__a, __b) __extension__ ({ \
    __m256d __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) \
        __r.d[__k] = (__u_a.d[__k] < __u_b.d[__k]) ? __u_a.d[__k] : __u_b.d[__k]; \
    __r; \
})

#define _mm256_max_pd(__a, __b) __extension__ ({ \
    __m256d __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) \
        __r.d[__k] = (__u_a.d[__k] > __u_b.d[__k]) ? __u_a.d[__k] : __u_b.d[__k]; \
    __r; \
})

/* Logical operations */

#define _mm256_and_ps(__a, __b) __extension__ ({ \
    __m256 __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.i[__k] = __u_a.i[__k] & __u_b.i[__k]; \
    __r; \
})

#define _mm256_andnot_ps(__a, __b) __extension__ ({ \
    __m256 __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.i[__k] = (~__u_a.i[__k]) & __u_b.i[__k]; \
    __r; \
})

#define _mm256_or_ps(__a, __b) __extension__ ({ \
    __m256 __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.i[__k] = __u_a.i[__k] | __u_b.i[__k]; \
    __r; \
})

#define _mm256_xor_ps(__a, __b) __extension__ ({ \
    __m256 __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.i[__k] = __u_a.i[__k] ^ __u_b.i[__k]; \
    __r; \
})

#define _mm256_and_pd(__a, __b) __extension__ ({ \
    __m256d __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.q[__k] = __u_a.q[__k] & __u_b.q[__k]; \
    __r; \
})

#define _mm256_andnot_pd(__a, __b) __extension__ ({ \
    __m256d __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.q[__k] = (~__u_a.q[__k]) & __u_b.q[__k]; \
    __r; \
})

#define _mm256_or_pd(__a, __b) __extension__ ({ \
    __m256d __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.q[__k] = __u_a.q[__k] | __u_b.q[__k]; \
    __r; \
})

#define _mm256_xor_pd(__a, __b) __extension__ ({ \
    __m256d __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.q[__k] = __u_a.q[__k] ^ __u_b.q[__k]; \
    __r; \
})

/* Load / Store */

static __inline__ __m256 __attribute__((__always_inline__))
_mm256_load_ps(float const *__p) {
    __m256 __r;
    memcpy(&__r, __p, sizeof(__m256));
    return __r;
}

static __inline__ __m256 __attribute__((__always_inline__))
_mm256_loadu_ps(float const *__p) {
    __m256 __r;
    memcpy(&__r, __p, sizeof(__m256));
    return __r;
}

static __inline__ __m256d __attribute__((__always_inline__))
_mm256_load_pd(double const *__p) {
    __m256d __r;
    memcpy(&__r, __p, sizeof(__m256d));
    return __r;
}

static __inline__ __m256d __attribute__((__always_inline__))
_mm256_loadu_pd(double const *__p) {
    __m256d __r;
    memcpy(&__r, __p, sizeof(__m256d));
    return __r;
}

static __inline__ __m256i __attribute__((__always_inline__))
_mm256_load_si256(__m256i const *__p) {
    __m256i __r;
    memcpy(&__r, __p, sizeof(__m256i));
    return __r;
}

static __inline__ __m256i __attribute__((__always_inline__))
_mm256_loadu_si256(__m256i const *__p) {
    __m256i __r;
    memcpy(&__r, __p, sizeof(__m256i));
    return __r;
}

#define _mm256_store_ps(__p, __a)   do { memcpy((__p), &(__a), sizeof(__m256)); } while(0)
#define _mm256_storeu_ps(__p, __a)  do { memcpy((__p), &(__a), sizeof(__m256)); } while(0)
#define _mm256_store_pd(__p, __a)   do { memcpy((__p), &(__a), sizeof(__m256d)); } while(0)
#define _mm256_storeu_pd(__p, __a)  do { memcpy((__p), &(__a), sizeof(__m256d)); } while(0)
#define _mm256_store_si256(__p, __a) do { memcpy((__p), &(__a), sizeof(__m256i)); } while(0)
#define _mm256_storeu_si256(__p, __a) do { memcpy((__p), &(__a), sizeof(__m256i)); } while(0)

/* Set and Broadcast */

#define _mm256_set1_ps(__w) __extension__ ({ \
    __m256 __r; float __val = (__w); int __k; \
    for (__k = 0; __k < 8; __k++) __r.f[__k] = __val; \
    __r; \
})

#define _mm256_set1_pd(__w) __extension__ ({ \
    __m256d __r; double __val = (__w); int __k; \
    for (__k = 0; __k < 4; __k++) __r.d[__k] = __val; \
    __r; \
})

#define _mm256_set_ps(__e7, __e6, __e5, __e4, __e3, __e2, __e1, __e0) __extension__ ({ \
    __m256 __r; \
    __r.f[0] = (__e0); __r.f[1] = (__e1); __r.f[2] = (__e2); __r.f[3] = (__e3); \
    __r.f[4] = (__e4); __r.f[5] = (__e5); __r.f[6] = (__e6); __r.f[7] = (__e7); \
    __r; \
})

#define _mm256_set_pd(__e3, __e2, __e1, __e0) __extension__ ({ \
    __m256d __r; \
    __r.d[0] = (__e0); __r.d[1] = (__e1); __r.d[2] = (__e2); __r.d[3] = (__e3); \
    __r; \
})

/* 128-bit lane extraction / insertion */

#define _mm256_extractf128_ps(__a, __imm) __extension__ ({ \
    __m256 __u = (__a); __m128 __r; \
    memcpy(&__r, &((__imm) & 1 ? __u.f[4] : __u.f[0]), sizeof(__m128)); \
    __r; \
})

#define _mm256_extractf128_pd(__a, __imm) __extension__ ({ \
    __m256d __u = (__a); __m128d __r; \
    memcpy(&__r, &((__imm) & 1 ? __u.d[2] : __u.d[0]), sizeof(__m128d)); \
    __r; \
})

#define _mm256_extractf128_si256(__a, __imm) __extension__ ({ \
    __m256i __u = (__a); __m128i __r; \
    memcpy(&__r, &((__imm) & 1 ? __u.q[2] : __u.q[0]), sizeof(__m128i)); \
    __r; \
})

/* Casts */

#define _mm256_castpd_ps(__a) (__a)
#define _mm256_castps_pd(__a) (__a)
#define _mm256_castps_si256(__a) (__a)
#define _mm256_castsi256_ps(__a) (__a)
#define _mm256_castpd_si256(__a) (__a)
#define _mm256_castsi256_pd(__a) (__a)

#endif /* _AVXINTRIN_H_INCLUDED */
