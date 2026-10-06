/* ======================================================================
 * pmmintrin.h — SSE3 intrinsics for TCC
 * ====================================================================== */

#ifndef _PMMINTRIN_H_INCLUDED
#define _PMMINTRIN_H_INCLUDED

#include "emmintrin.h"

/* Floating point arithmetic */

static __inline__ __m128 __attribute__((__always_inline__))
_mm_addsub_ps(__m128 __a, __m128 __b) {
    __m128 __r;
    __r.f[0] = __a.f[0] - __b.f[0];
    __r.f[1] = __a.f[1] + __b.f[1];
    __r.f[2] = __a.f[2] - __b.f[2];
    __r.f[3] = __a.f[3] + __b.f[3];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_addsub_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.d[0] = __a.d[0] - __b.d[0];
    __r.d[1] = __a.d[1] + __b.d[1];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_hadd_ps(__m128 __a, __m128 __b) {
    __m128 __r;
    __r.f[0] = __a.f[0] + __a.f[1];
    __r.f[1] = __a.f[2] + __a.f[3];
    __r.f[2] = __b.f[0] + __b.f[1];
    __r.f[3] = __b.f[2] + __b.f[3];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_hadd_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.d[0] = __a.d[0] + __a.d[1];
    __r.d[1] = __b.d[0] + __b.d[1];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_hsub_ps(__m128 __a, __m128 __b) {
    __m128 __r;
    __r.f[0] = __a.f[0] - __a.f[1];
    __r.f[1] = __a.f[2] - __a.f[3];
    __r.f[2] = __b.f[0] - __b.f[1];
    __r.f[3] = __b.f[2] - __b.f[3];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_hsub_pd(__m128d __a, __m128d __b) {
    __m128d __r;
    __r.d[0] = __a.d[0] - __a.d[1];
    __r.d[1] = __b.d[0] - __b.d[1];
    return __r;
}

/* Duplication & load */

static __inline__ __m128d __attribute__((__always_inline__))
_mm_movedup_pd(__m128d __a) {
    __m128d __r;
    __r.d[0] = __a.d[0]; __r.d[1] = __a.d[0];
    return __r;
}

static __inline__ __m128d __attribute__((__always_inline__))
_mm_loaddup_pd(double const *__dp) {
    __m128d __r;
    __r.d[0] = *__dp; __r.d[1] = *__dp;
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_movehdup_ps(__m128 __a) {
    __m128 __r;
    __r.f[0] = __a.f[1]; __r.f[1] = __a.f[1];
    __r.f[2] = __a.f[3]; __r.f[3] = __a.f[3];
    return __r;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_moveldup_ps(__m128 __a) {
    __m128 __r;
    __r.f[0] = __a.f[0]; __r.f[1] = __a.f[0];
    __r.f[2] = __a.f[2]; __r.f[3] = __a.f[2];
    return __r;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_lddqu_si128(__m128i const *__p) {
    return _mm_loadu_si128(__p);
}

/* Thread synchronization */

static __inline__ void __attribute__((__always_inline__))
_mm_monitor(void const *__p, unsigned __extensions, unsigned __hints) {
    __asm__ __volatile__("monitor" : : "a"(__p), "c"(__extensions), "d"(__hints));
}

static __inline__ void __attribute__((__always_inline__))
_mm_mwait(unsigned __extensions, unsigned __hints) {
    __asm__ __volatile__("mwait" : : "a"(__hints), "c"(__extensions));
}

#endif /* _PMMINTRIN_H_INCLUDED */
