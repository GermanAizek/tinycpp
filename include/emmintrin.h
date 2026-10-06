/* ======================================================================
 * emmintrin.h — SSE2 intrinsics for TCC
 * ====================================================================== */

#ifndef _EMMINTRIN_H_INCLUDED
#define _EMMINTRIN_H_INCLUDED

#include "xmmintrin.h"

typedef __m128 __m128d;
typedef __m128 __m128i;

/* Double-precision floating point arithmetic */

static __inline__ __m128d __attribute__((__always_inline__))
_mm_add_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.d[0] = __a.d[0] + __b.d[0];
    __r.d[1] = __a.d[1] + __b.d[1];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_add_sd(__m128d __a, __m128d __b) {
    __m128d __r = __a;
    __r.d[0] = __a.d[0] + __b.d[0];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_sub_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.d[0] = __a.d[0] - __b.d[0];
    __r.d[1] = __a.d[1] - __b.d[1];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_sub_sd(__m128d __a, __m128d __b) {
    __m128d __r = __a;
    __r.d[0] = __a.d[0] - __b.d[0];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_mul_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.d[0] = __a.d[0] * __b.d[0];
    __r.d[1] = __a.d[1] * __b.d[1];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_mul_sd(__m128d __a, __m128d __b) {
    __m128d __r = __a;
    __r.d[0] = __a.d[0] * __b.d[0];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_div_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.d[0] = __a.d[0] / __b.d[0];
    __r.d[1] = __a.d[1] / __b.d[1];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_div_sd(__m128d __a, __m128d __b) {
    __m128d __r = __a;
    __r.d[0] = __a.d[0] / __b.d[0];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_sqrt_pd(__m128d __a) {
    __m128d __r;
    __r.d[0] = sqrt(__a.d[0]);
    __r.d[1] = sqrt(__a.d[1]);
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_sqrt_sd(__m128d __a) {
    __m128d __r = __a;
    __r.d[0] = sqrt(__a.d[0]);
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_min_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.d[0] = (__a.d[0] < __b.d[0]) ? __a.d[0] : __b.d[0];
    __r.d[1] = (__a.d[1] < __b.d[1]) ? __a.d[1] : __b.d[1];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_max_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.d[0] = (__a.d[0] > __b.d[0]) ? __a.d[0] : __b.d[0];
    __r.d[1] = (__a.d[1] > __b.d[1]) ? __a.d[1] : __b.d[1];
    return __r;
}

/* Logical operations */

static __inline__ __m128d __attribute__((__always_inline__))
_mm_and_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.q[0] = __a.q[0] & __b.q[0];
    __r.q[1] = __a.q[1] & __b.q[1];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_andnot_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.q[0] = (~__a.q[0]) & __b.q[0];
    __r.q[1] = (~__a.q[1]) & __b.q[1];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_or_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.q[0] = __a.q[0] | __b.q[0];
    __r.q[1] = __a.q[1] | __b.q[1];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_xor_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.q[0] = __a.q[0] ^ __b.q[0];
    __r.q[1] = __a.q[1] ^ __b.q[1];
    return __r;
}

/* Comparisons */

static __inline__ __m128d __attribute__((__always_inline__))
_mm_cmpeq_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.q[0] = (__a.d[0] == __b.d[0]) ? -1LL : 0LL;
    __r.q[1] = (__a.d[1] == __b.d[1]) ? -1LL : 0LL;
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_cmplt_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.q[0] = (__a.d[0] < __b.d[0]) ? -1LL : 0LL;
    __r.q[1] = (__a.d[1] < __b.d[1]) ? -1LL : 0LL;
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_cmple_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.q[0] = (__a.d[0] <= __b.d[0]) ? -1LL : 0LL;
    __r.q[1] = (__a.d[1] <= __b.d[1]) ? -1LL : 0LL;
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_cmpgt_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.q[0] = (__a.d[0] > __b.d[0]) ? -1LL : 0LL;
    __r.q[1] = (__a.d[1] > __b.d[1]) ? -1LL : 0LL;
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_cmpge_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.q[0] = (__a.d[0] >= __b.d[0]) ? -1LL : 0LL;
    __r.q[1] = (__a.d[1] >= __b.d[1]) ? -1LL : 0LL;
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_cmpneq_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.q[0] = (__a.d[0] != __b.d[0]) ? -1LL : 0LL;
    __r.q[1] = (__a.d[1] != __b.d[1]) ? -1LL : 0LL;
    return __r;
}

/* Double Load / Store / Set */

static __inline__ __m128d __attribute__((__always_inline__))
_mm_setzero_pd(void) {
    __m128d __r;
    __r.d[0] = 0.0; __r.d[1] = 0.0;
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_set1_pd(double __w) {
    __m128d __r;
    __r.d[0] = __w; __r.d[1] = __w;
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_set_pd(double __e1, double __e0) {
    __m128d __r;
    __r.d[0] = __e0; __r.d[1] = __e1;
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_setr_pd(double __e0, double __e1) {
    return _mm_set_pd(__e1, __e0);
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_set_sd(double __w) {
    __m128d __r;
    __r.d[0] = __w; __r.d[1] = 0.0;
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_load_pd(double const *__p) {
    __m128d __r;
    memcpy(&__r, __p, sizeof(__m128d));
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_loadu_pd(double const *__p) {
    __m128d __r;
    memcpy(&__r, __p, sizeof(__m128d));
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_load_sd(double const *__p) {
    __m128d __r = _mm_setzero_pd();
    __r.d[0] = *__p;
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_load1_pd(double const *__p) {
    return _mm_set1_pd(*__p);
}

static __inline__ void __attribute__((__always_inline__))
_mm_store_pd(double *__p, __m128d __a) {
    memcpy(__p, &__a, sizeof(__m128d));
}

static __inline__ void __attribute__((__always_inline__))
_mm_storeu_pd(double *__p, __m128d __a) {
    memcpy(__p, &__a, sizeof(__m128d));
}

static __inline__ void __attribute__((__always_inline__))
_mm_store_sd(double *__p, __m128d __a) {
    *__p = __a.d[0];
}

static __inline__ void __attribute__((__always_inline__))
_mm_store1_pd(double *__p, __m128d __a) {
    __p[0] = __a.d[0]; __p[1] = __a.d[0];
}

/* Integer SIMD arithmetic */

static __inline__ __m128i __attribute__((__always_inline__))
_mm_add_epi8(__m128i __a, __m128i __b) {
    __m128i __r; int __k;
    for (__k = 0; __k < 16; __k++) __r.b[__k] = __a.b[__k] + __b.b[__k];
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_add_epi16(__m128i __a, __m128i __b) {
    __m128i __r; int __k;
    for (__k = 0; __k < 8; __k++) __r.s[__k] = __a.s[__k] + __b.s[__k];
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_add_epi32(__m128i __a, __m128i __b) {
    __m128i __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.i[__k] = __a.i[__k] + __b.i[__k];
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_add_epi64(__m128i __a, __m128i __b) {
    __m128i __r; int __k;
    for (__k = 0; __k < 2; __k++) __r.q[__k] = __a.q[__k] + __b.q[__k];
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_sub_epi8(__m128i __a, __m128i __b) {
    __m128i __r; int __k;
    for (__k = 0; __k < 16; __k++) __r.b[__k] = __a.b[__k] - __b.b[__k];
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_sub_epi16(__m128i __a, __m128i __b) {
    __m128i __r; int __k;
    for (__k = 0; __k < 8; __k++) __r.s[__k] = __a.s[__k] - __b.s[__k];
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_sub_epi32(__m128i __a, __m128i __b) {
    __m128i __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.i[__k] = __a.i[__k] - __b.i[__k];
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_sub_epi64(__m128i __a, __m128i __b) {
    __m128i __r; int __k;
    for (__k = 0; __k < 2; __k++) __r.q[__k] = __a.q[__k] - __b.q[__k];
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_mullo_epi16(__m128i __a, __m128i __b) {
    __m128i __r; int __k;
    for (__k = 0; __k < 8; __k++) __r.s[__k] = __a.s[__k] * __b.s[__k];
    return __r;
}

/* Bitwise integer logic */

static __inline__ __m128i __attribute__((__always_inline__))
_mm_and_si128(__m128i __a, __m128i __b) {
    __m128i __r;
    __r.q[0] = __a.q[0] & __b.q[0];
    __r.q[1] = __a.q[1] & __b.q[1];
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_andnot_si128(__m128i __a, __m128i __b) {
    __m128i __r;
    __r.q[0] = (~__a.q[0]) & __b.q[0];
    __r.q[1] = (~__a.q[1]) & __b.q[1];
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_or_si128(__m128i __a, __m128i __b) {
    __m128i __r;
    __r.q[0] = __a.q[0] | __b.q[0];
    __r.q[1] = __a.q[1] | __b.q[1];
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_xor_si128(__m128i __a, __m128i __b) {
    __m128i __r;
    __r.q[0] = __a.q[0] ^ __b.q[0];
    __r.q[1] = __a.q[1] ^ __b.q[1];
    return __r;
}

/* Integer comparisons */

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cmpeq_epi8(__m128i __a, __m128i __b) {
    __m128i __r; int __k;
    for (__k = 0; __k < 16; __k++) __r.b[__k] = (__a.b[__k] == __b.b[__k]) ? -1 : 0;
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cmpeq_epi16(__m128i __a, __m128i __b) {
    __m128i __r; int __k;
    for (__k = 0; __k < 8; __k++) __r.s[__k] = (__a.s[__k] == __b.s[__k]) ? -1 : 0;
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cmpeq_epi32(__m128i __a, __m128i __b) {
    __m128i __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.i[__k] = (__a.i[__k] == __b.i[__k]) ? -1 : 0;
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cmpgt_epi8(__m128i __a, __m128i __b) {
    __m128i __r; int __k;
    for (__k = 0; __k < 16; __k++) __r.b[__k] = (__a.b[__k] > __b.b[__k]) ? -1 : 0;
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cmpgt_epi16(__m128i __a, __m128i __b) {
    __m128i __r; int __k;
    for (__k = 0; __k < 8; __k++) __r.s[__k] = (__a.s[__k] > __b.s[__k]) ? -1 : 0;
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cmpgt_epi32(__m128i __a, __m128i __b) {
    __m128i __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.i[__k] = (__a.i[__k] > __b.i[__k]) ? -1 : 0;
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cmplt_epi8(__m128i __a, __m128i __b) {
    return _mm_cmpgt_epi8(__b, __a);
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cmplt_epi16(__m128i __a, __m128i __b) {
    return _mm_cmpgt_epi16(__b, __a);
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cmplt_epi32(__m128i __a, __m128i __b) {
    return _mm_cmpgt_epi32(__b, __a);
}

/* Integer Shifts */

#define _mm_slli_epi16(__a, __count) __extension__ ({ \
    __m128i __u = (__a), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.s[__k] = (__count < 16) ? (__u.s[__k] << (__count)) : 0; \
    __r; \
})

#define _mm_slli_epi32(__a, __count) __extension__ ({ \
    __m128i __u = (__a), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.i[__k] = (__count < 32) ? (__u.i[__k] << (__count)) : 0; \
    __r; \
})

#define _mm_slli_epi64(__a, __count) __extension__ ({ \
    __m128i __u = (__a), __r; int __k; \
    for (__k = 0; __k < 2; __k++) __r.q[__k] = (__count < 64) ? (__u.q[__k] << (__count)) : 0; \
    __r; \
})

#define _mm_srli_epi16(__a, __count) __extension__ ({ \
    __m128i __u = (__a), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.us[__k] = (__count < 16) ? (__u.us[__k] >> (__count)) : 0; \
    __r; \
})

#define _mm_srli_epi32(__a, __count) __extension__ ({ \
    __m128i __u = (__a), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.ui[__k] = (__count < 32) ? (__u.ui[__k] >> (__count)) : 0; \
    __r; \
})

#define _mm_srli_epi64(__a, __count) __extension__ ({ \
    __m128i __u = (__a), __r; int __k; \
    for (__k = 0; __k < 2; __k++) __r.uq[__k] = (__count < 64) ? (__u.uq[__k] >> (__count)) : 0; \
    __r; \
})

#define _mm_srai_epi16(__a, __count) __extension__ ({ \
    __m128i __u = (__a), __r; int __k; int __c = (__count > 15) ? 15 : (__count); \
    for (__k = 0; __k < 8; __k++) __r.s[__k] = __u.s[__k] >> __c; \
    __r; \
})

#define _mm_srai_epi32(__a, __count) __extension__ ({ \
    __m128i __u = (__a), __r; int __k; int __c = (__count > 31) ? 31 : (__count); \
    for (__k = 0; __k < 4; __k++) __r.i[__k] = __u.i[__k] >> __c; \
    __r; \
})

/* Integer Set / Load / Store */

static __inline__ __m128i __attribute__((__always_inline__))
_mm_setzero_si128(void) {
    __m128i __r;
    __r.q[0] = 0LL; __r.q[1] = 0LL;
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_set1_epi8(char __w) {
    __m128i __r; int __k;
    for (__k = 0; __k < 16; __k++) __r.b[__k] = __w;
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_set1_epi16(short __w) {
    __m128i __r; int __k;
    for (__k = 0; __k < 8; __k++) __r.s[__k] = __w;
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_set1_epi32(int __w) {
    __m128i __r; int __k;
    for (__k = 0; __k < 4; __k++) __r.i[__k] = __w;
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_set1_epi64x(long long __w) {
    __m128i __r;
    __r.q[0] = __w; __r.q[1] = __w;
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_set_epi32(int __e3, int __e2, int __e1, int __e0) {
    __m128i __r;
    __r.i[0] = __e0; __r.i[1] = __e1; __r.i[2] = __e2; __r.i[3] = __e3;
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_set_epi64x(long long __e1, long long __e0) {
    __m128i __r;
    __r.q[0] = __e0; __r.q[1] = __e1;
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_load_si128(__m128i const *__p) {
    __m128i __r;
    memcpy(&__r, __p, sizeof(__m128i));
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_loadu_si128(__m128i const *__p) {
    __m128i __r;
    memcpy(&__r, __p, sizeof(__m128i));
    return __r;
}

static __inline__ void __attribute__((__always_inline__))
_mm_store_si128(__m128i *__p, __m128i __a) {
    memcpy(__p, &__a, sizeof(__m128i));
}

static __inline__ void __attribute__((__always_inline__))
_mm_storeu_si128(__m128i *__p, __m128i __a) {
    memcpy(__p, &__a, sizeof(__m128i));
}

/* Shuffles & Packs */

#define _mm_shuffle_epi32(__a, __imm) __extension__ ({ \
    __m128i __u = (__a), __r; \
    __r.i[0] = __u.i[((__imm)     ) & 3]; \
    __r.i[1] = __u.i[((__imm) >> 2) & 3]; \
    __r.i[2] = __u.i[((__imm) >> 4) & 3]; \
    __r.i[3] = __u.i[((__imm) >> 6) & 3]; \
    __r; \
})

#define _mm_shuffle_pd(__a, __b, __mask) __extension__ ({ \
    __m128d __u_a = (__a), __u_b = (__b), __r; \
    __r.d[0] = ((__mask) & 1) ? __u_a.d[1] : __u_a.d[0]; \
    __r.d[1] = ((__mask) & 2) ? __u_b.d[1] : __u_b.d[0]; \
    __r; \
})

/* Casts */

static __inline__ __m128  __attribute__((__always_inline__)) _mm_castpd_ps(__m128d __a) { return __a; }
static __inline__ __m128d __attribute__((__always_inline__)) _mm_castps_pd(__m128  __a) { return __a; }
static __inline__ __m128i __attribute__((__always_inline__)) _mm_castps_si128(__m128 __a) { return __a; }
static __inline__ __m128  __attribute__((__always_inline__)) _mm_castsi128_ps(__m128i __a) { return __a; }
static __inline__ __m128i __attribute__((__always_inline__)) _mm_castpd_si128(__m128d __a) { return __a; }
static __inline__ __m128d __attribute__((__always_inline__)) _mm_castsi128_pd(__m128i __a) { return __a; }

/* Fences */

static __inline__ void __attribute__((__always_inline__))
_mm_clflush(void const *__p) {
    __asm__ __volatile__("clflush %0" : : "m"(*(char const *)__p));
}

static __inline__ void __attribute__((__always_inline__))
_mm_lfence(void) {
    __asm__ __volatile__("lfence" ::: "memory");
}

static __inline__ void __attribute__((__always_inline__))
_mm_mfence(void) {
    __asm__ __volatile__("mfence" ::: "memory");
}

#endif /* _EMMINTRIN_H_INCLUDED */
