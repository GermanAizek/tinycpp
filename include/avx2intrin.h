/* ======================================================================
 * avx2intrin.h — Advanced Vector Extensions 2 (AVX2) intrinsics for TCC
 * ====================================================================== */

#ifndef _AVX2INTRIN_H_INCLUDED
#define _AVX2INTRIN_H_INCLUDED

#include "avxintrin.h"

/* 256-bit integer vector arithmetic */

#define _mm256_add_epi8(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 32; __k++) __r.b[__k] = __u_a.b[__k] + __u_b.b[__k]; \
    __r; \
})

#define _mm256_add_epi16(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 16; __k++) __r.s[__k] = __u_a.s[__k] + __u_b.s[__k]; \
    __r; \
})

#define _mm256_add_epi32(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.i[__k] = __u_a.i[__k] + __u_b.i[__k]; \
    __r; \
})

#define _mm256_add_epi64(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.q[__k] = __u_a.q[__k] + __u_b.q[__k]; \
    __r; \
})

#define _mm256_sub_epi8(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 32; __k++) __r.b[__k] = __u_a.b[__k] - __u_b.b[__k]; \
    __r; \
})

#define _mm256_sub_epi16(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 16; __k++) __r.s[__k] = __u_a.s[__k] - __u_b.s[__k]; \
    __r; \
})

#define _mm256_sub_epi32(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.i[__k] = __u_a.i[__k] - __u_b.i[__k]; \
    __r; \
})

#define _mm256_sub_epi64(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.q[__k] = __u_a.q[__k] - __u_b.q[__k]; \
    __r; \
})

#define _mm256_mullo_epi16(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 16; __k++) __r.s[__k] = __u_a.s[__k] * __u_b.s[__k]; \
    __r; \
})

#define _mm256_mullo_epi32(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.i[__k] = __u_a.i[__k] * __u_b.i[__k]; \
    __r; \
})

/* 256-bit bitwise logic */

#define _mm256_and_si256(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.q[__k] = __u_a.q[__k] & __u_b.q[__k]; \
    __r; \
})

#define _mm256_andnot_si256(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.q[__k] = (~__u_a.q[__k]) & __u_b.q[__k]; \
    __r; \
})

#define _mm256_or_si256(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.q[__k] = __u_a.q[__k] | __u_b.q[__k]; \
    __r; \
})

#define _mm256_xor_si256(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.q[__k] = __u_a.q[__k] ^ __u_b.q[__k]; \
    __r; \
})

/* Comparisons */

#define _mm256_cmpeq_epi8(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 32; __k++) __r.b[__k] = (__u_a.b[__k] == __u_b.b[__k]) ? -1 : 0; \
    __r; \
})

#define _mm256_cmpeq_epi16(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 16; __k++) __r.s[__k] = (__u_a.s[__k] == __u_b.s[__k]) ? -1 : 0; \
    __r; \
})

#define _mm256_cmpeq_epi32(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.i[__k] = (__u_a.i[__k] == __u_b.i[__k]) ? -1 : 0; \
    __r; \
})

#define _mm256_cmpeq_epi64(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.q[__k] = (__u_a.q[__k] == __u_b.q[__k]) ? -1LL : 0LL; \
    __r; \
})

#define _mm256_cmpgt_epi8(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 32; __k++) __r.b[__k] = (__u_a.b[__k] > __u_b.b[__k]) ? -1 : 0; \
    __r; \
})

#define _mm256_cmpgt_epi16(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 16; __k++) __r.s[__k] = (__u_a.s[__k] > __u_b.s[__k]) ? -1 : 0; \
    __r; \
})

#define _mm256_cmpgt_epi32(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.i[__k] = (__u_a.i[__k] > __u_b.i[__k]) ? -1 : 0; \
    __r; \
})

#define _mm256_cmpgt_epi64(__a, __b) __extension__ ({ \
    __m256i __u_a = (__a), __u_b = (__b), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.q[__k] = (__u_a.q[__k] > __u_b.q[__k]) ? -1LL : 0LL; \
    __r; \
})

/* Shifts */

#define _mm256_slli_epi16(__a, __count) __extension__ ({ \
    __m256i __u = (__a), __r; int __k; \
    for (__k = 0; __k < 16; __k++) __r.s[__k] = (__count < 16) ? (__u.s[__k] << (__count)) : 0; \
    __r; \
})

#define _mm256_slli_epi32(__a, __count) __extension__ ({ \
    __m256i __u = (__a), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.i[__k] = (__count < 32) ? (__u.i[__k] << (__count)) : 0; \
    __r; \
})

#define _mm256_slli_epi64(__a, __count) __extension__ ({ \
    __m256i __u = (__a), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.q[__k] = (__count < 64) ? (__u.q[__k] << (__count)) : 0; \
    __r; \
})

#define _mm256_srli_epi16(__a, __count) __extension__ ({ \
    __m256i __u = (__a), __r; int __k; \
    for (__k = 0; __k < 16; __k++) __r.us[__k] = (__count < 16) ? (__u.us[__k] >> (__count)) : 0; \
    __r; \
})

#define _mm256_srli_epi32(__a, __count) __extension__ ({ \
    __m256i __u = (__a), __r; int __k; \
    for (__k = 0; __k < 8; __k++) __r.ui[__k] = (__count < 32) ? (__u.ui[__k] >> (__count)) : 0; \
    __r; \
})

#define _mm256_srli_epi64(__a, __count) __extension__ ({ \
    __m256i __u = (__a), __r; int __k; \
    for (__k = 0; __k < 4; __k++) __r.uq[__k] = (__count < 64) ? (__u.uq[__k] >> (__count)) : 0; \
    __r; \
})

/* Set and Broadcast */

#define _mm256_set1_epi8(__w) __extension__ ({ \
    __m256i __r; char __val = (__w); int __k; \
    for (__k = 0; __k < 32; __k++) __r.b[__k] = __val; \
    __r; \
})

#define _mm256_set1_epi16(__w) __extension__ ({ \
    __m256i __r; short __val = (__w); int __k; \
    for (__k = 0; __k < 16; __k++) __r.s[__k] = __val; \
    __r; \
})

#define _mm256_set1_epi32(__w) __extension__ ({ \
    __m256i __r; int __val = (__w); int __k; \
    for (__k = 0; __k < 8; __k++) __r.i[__k] = __val; \
    __r; \
})

#define _mm256_set1_epi64x(__w) __extension__ ({ \
    __m256i __r; long long __val = (__w); int __k; \
    for (__k = 0; __k < 4; __k++) __r.q[__k] = __val; \
    __r; \
})

#define _mm256_set_epi32(__e7, __e6, __e5, __e4, __e3, __e2, __e1, __e0) __extension__ ({ \
    __m256i __r; \
    __r.i[0] = (__e0); __r.i[1] = (__e1); __r.i[2] = (__e2); __r.i[3] = (__e3); \
    __r.i[4] = (__e4); __r.i[5] = (__e5); __r.i[6] = (__e6); __r.i[7] = (__e7); \
    __r; \
})

#define _mm256_set_epi64x(__e3, __e2, __e1, __e0) __extension__ ({ \
    __m256i __r; \
    __r.q[0] = (__e0); __r.q[1] = (__e1); __r.q[2] = (__e2); __r.q[3] = (__e3); \
    __r; \
})

#endif /* _AVX2INTRIN_H_INCLUDED */
