/* ======================================================================
 * immintrin.h — Master SIMD / Vector intrinsics header for TCC
 * ====================================================================== */

#ifndef _IMMINTRIN_H_INCLUDED
#define _IMMINTRIN_H_INCLUDED

#include "xmmintrin.h"
#include "emmintrin.h"
#include "pmmintrin.h"
#include "tmmintrin.h"
#include "smmintrin.h"
#include "nmmintrin.h"
#include "wmmintrin.h"
#include "avxintrin.h"
#include "avx2intrin.h"
#include "fmaintrin.h"

/* AVX-512 Foundation Types */

typedef union {
    float              f[16];
    double             d[8];
    signed char        b[64];
    short              s[32];
    int                i[16];
    long long          q[8];
    unsigned char      ub[64];
    unsigned short     us[32];
    unsigned int       ui[16];
    unsigned long long uq[8];
    float              m512_f32[16];
    double             m512_f64[8];
    signed char        m512_i8[64];
    short              m512_i16[32];
    int                m512_i32[16];
    long long          m512_i64[8];
    unsigned char      m512_u8[64];
    unsigned short     m512_u16[32];
    unsigned int       m512_u32[16];
    unsigned long long m512_u64[8];
} __attribute__((__aligned__(64))) __m512;

typedef __m512 __m512d;
typedef __m512 __m512i;

typedef unsigned char      __mmask8;
typedef unsigned short     __mmask16;
typedef unsigned int       __mmask32;
typedef unsigned long long __mmask64;

/* AVX-512 Zero initialization */

#define _mm512_setzero_ps() __extension__ ({ \
    __m512 __r; int __k; \
    for (__k = 0; __k < 16; __k++) __r.f[__k] = 0.0f; \
    __r; \
})

#define _mm512_setzero_pd() __extension__ ({ \
    __m512d __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.d[__k] = 0.0; \
    __r; \
})

#define _mm512_setzero_si512() __extension__ ({ \
    __m512i __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.q[__k] = 0LL; \
    __r; \
})

/* AVX-512 Single-precision floating point arithmetic */

#define _mm512_add_ps(__a, __b) __extension__ ({ \
    __m512 __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 16; __k++) __r.f[__k] = __u_a.f[__k] + __u_b.f[__k]; \
    __r; \
})

#define _mm512_sub_ps(__a, __b) __extension__ ({ \
    __m512 __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 16; __k++) __r.f[__k] = __u_a.f[__k] - __u_b.f[__k]; \
    __r; \
})

#define _mm512_mul_ps(__a, __b) __extension__ ({ \
    __m512 __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 16; __k++) __r.f[__k] = __u_a.f[__k] * __u_b.f[__k]; \
    __r; \
})

#define _mm512_div_ps(__a, __b) __extension__ ({ \
    __m512 __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 16; __k++) __r.f[__k] = __u_a.f[__k] / __u_b.f[__k]; \
    __r; \
})

/* AVX-512 Double-precision floating point arithmetic */

#define _mm512_add_pd(__a, __b) __extension__ ({ \
    __m512d __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.d[__k] = __u_a.d[__k] + __u_b.d[__k]; \
    __r; \
})

#define _mm512_sub_pd(__a, __b) __extension__ ({ \
    __m512d __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.d[__k] = __u_a.d[__k] - __u_b.d[__k]; \
    __r; \
})

#define _mm512_mul_pd(__a, __b) __extension__ ({ \
    __m512d __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.d[__k] = __u_a.d[__k] * __u_b.d[__k]; \
    __r; \
})

#define _mm512_div_pd(__a, __b) __extension__ ({ \
    __m512d __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.d[__k] = __u_a.d[__k] / __u_b.d[__k]; \
    __r; \
})

/* AVX-512 Load / Store */

#define _mm512_load_ps(__p) __extension__ ({ \
    __m512 __r; \
    memcpy(&__r, (__p), sizeof(__m512)); \
    __r; \
})

#define _mm512_loadu_ps(__p) _mm512_load_ps(__p)

#define _mm512_store_ps(__p, __a)   do { memcpy((__p), &(__a), sizeof(__m512)); } while(0)
#define _mm512_storeu_ps(__p, __a)  do { memcpy((__p), &(__a), sizeof(__m512)); } while(0)

#define _mm512_load_pd(__p) __extension__ ({ \
    __m512d __r; \
    memcpy(&__r, (__p), sizeof(__m512d)); \
    __r; \
})

#define _mm512_loadu_pd(__p) _mm512_load_pd(__p)

#define _mm512_store_pd(__p, __a)   do { memcpy((__p), &(__a), sizeof(__m512d)); } while(0)
#define _mm512_storeu_pd(__p, __a)  do { memcpy((__p), &(__a), sizeof(__m512d)); } while(0)

#define _mm512_load_si512(__p) __extension__ ({ \
    __m512i __r; \
    memcpy(&__r, (__p), sizeof(__m512i)); \
    __r; \
})

#define _mm512_loadu_si512(__p) _mm512_load_si512(__p)

#define _mm512_store_si512(__p, __a)  do { memcpy((__p), &(__a), sizeof(__m512i)); } while(0)
#define _mm512_storeu_si512(__p, __a) do { memcpy((__p), &(__a), sizeof(__m512i)); } while(0)

/* AVX-512 Broadcast / Set */

#define _mm512_set1_ps(__w) __extension__ ({ \
    float __u_w = (__w); __m512 __r; int __k; \
    for (__k = 0; __k < 16; __k++) __r.f[__k] = __u_w; \
    __r; \
})

#define _mm512_set1_pd(__w) __extension__ ({ \
    double __u_w = (__w); __m512d __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.d[__k] = __u_w; \
    __r; \
})

#define _mm512_set1_epi32(__w) __extension__ ({ \
    int __u_w = (__w); __m512i __r; int __k; \
    for (__k = 0; __k < 16; __k++) __r.i[__k] = __u_w; \
    __r; \
})

#define _mm512_set1_epi64(__w) __extension__ ({ \
    long long __u_w = (__w); __m512i __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.q[__k] = __u_w; \
    __r; \
})

#endif /* _IMMINTRIN_H_INCLUDED */
