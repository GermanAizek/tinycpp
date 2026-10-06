#include <stdio.h>
#include <string.h>
#include "immintrin.h"

int test_sse(void) {
    __m128 a = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 b = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 c = _mm_add_ps(a, b);
    float res[4];
    _mm_storeu_ps(res, c);
    for (int i = 0; i < 4; i++) {
        if (res[i] != 5.0f) {
            printf("SSE failed at %d: got %f, expected 5.0\n", i, res[i]);
            return 1;
        }
    }
    printf("SSE [PASS]\n");
    return 0;
}

int test_sse2(void) {
    __m128d a = _mm_set_pd(2.0, 1.0);
    __m128d b = _mm_set_pd(4.0, 3.0);
    __m128d c = _mm_mul_pd(a, b);
    double res[2];
    _mm_storeu_pd(res, c);
    if (res[0] != 3.0 || res[1] != 8.0) {
        printf("SSE2 failed: got [%f, %f]\n", res[0], res[1]);
        return 1;
    }
    printf("SSE2 [PASS]\n");
    return 0;
}

int test_sse3(void) {
    __m128 a = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 b = _mm_set_ps(1.0f, 1.0f, 1.0f, 1.0f);
    __m128 c = _mm_addsub_ps(a, b);
    float res[4];
    _mm_storeu_ps(res, c);
    /* addsub: res[0]=a0-b0, res[1]=a1+b1, res[2]=a2-b2, res[3]=a3+b3 */
    if (res[0] != 0.0f || res[1] != 3.0f || res[2] != 2.0f || res[3] != 5.0f) {
        printf("SSE3 failed: got [%f, %f, %f, %f]\n", res[0], res[1], res[2], res[3]);
        return 1;
    }
    printf("SSE3 [PASS]\n");
    return 0;
}

int test_ssse3(void) {
    __m128i a = _mm_set1_epi32(-42);
    __m128i b = _mm_abs_epi32(a);
    union { __m128i v; int i[4]; } u;
    u.v = b;
    for (int k = 0; k < 4; k++) {
        if (u.i[k] != 42) {
            printf("SSSE3 abs failed at %d: got %d\n", k, u.i[k]);
            return 1;
        }
    }
    printf("SSSE3 [PASS]\n");
    return 0;
}

int test_sse4(void) {
    __m128i a = _mm_set1_epi32(10);
    __m128i b = _mm_set1_epi32(20);
    __m128i min_val = _mm_min_epi32(a, b);
    __m128i max_val = _mm_max_epi32(a, b);
    union { __m128i v; int i[4]; } u_min, u_max;
    u_min.v = min_val;
    u_max.v = max_val;
    if (u_min.i[0] != 10 || u_max.i[0] != 20) {
        printf("SSE4 min/max failed\n");
        return 1;
    }
    unsigned int crc = _mm_crc32_u8(0, 'A');
    if (crc == 0) {
        printf("SSE4.2 crc32 failed\n");
        return 1;
    }
    printf("SSE4 [PASS]\n");
    return 0;
}

int test_avx(void) {
    _mm256_zeroupper();
    __m256 a = _mm256_set1_ps(3.5f);
    __m256 b = _mm256_set1_ps(2.0f);
    __m256 c = _mm256_add_ps(a, b);
    float res[8];
    _mm256_storeu_ps(res, c);
    for (int i = 0; i < 8; i++) {
        if (res[i] != 5.5f) {
            printf("AVX add_ps failed at %d: got %f\n", i, res[i]);
            return 1;
        }
    }
    _mm256_zeroupper();
    printf("AVX [PASS]\n");
    return 0;
}

int test_avx2(void) {
    __m256i a = _mm256_set1_epi32(100);
    __m256i b = _mm256_set1_epi32(25);
    __m256i c = _mm256_add_epi32(a, b);
    __m256i d = _mm256_sub_epi32(a, b);
    union { __m256i v; int i[8]; } uc, ud;
    uc.v = c;
    ud.v = d;
    for (int k = 0; k < 8; k++) {
        if (uc.i[k] != 125 || ud.i[k] != 75) {
            printf("AVX2 add/sub failed at %d: got [%d, %d]\n", k, uc.i[k], ud.i[k]);
            return 1;
        }
    }
    printf("AVX2 [PASS]\n");
    return 0;
}

int test_fma(void) {
    __m128 a = _mm_set1_ps(2.0f);
    __m128 b = _mm_set1_ps(3.0f);
    __m128 c = _mm_set1_ps(4.0f);
    /* fmadd computes a * b + c = 2 * 3 + 4 = 10 */
    __m128 r = _mm_fmadd_ps(a, b, c);
    float res[4];
    _mm_storeu_ps(res, r);
    for (int i = 0; i < 4; i++) {
        if (res[i] != 10.0f) {
            printf("FMA3 fmadd failed at %d: got %f, expected 10.0\n", i, res[i]);
            return 1;
        }
    }
    printf("FMA3 [PASS]\n");
    return 0;
}

int test_aes_clmul(void) {
    __m128i key = _mm_setzero_si128();
    __m128i val = _mm_set1_epi8(0x01);
    __m128i enc = _mm_aesenc_si128(val, key);
    if (enc.ub[0] == 0) {
        printf("AES enc failed\n");
        return 1;
    }
    __m128i a = _mm_set_epi64x(0x2, 0x3);
    __m128i b = _mm_set_epi64x(0x4, 0x5);
    /* Carry-less multiplication of 3 and 5 = 1101b = 15 or 3*5 without carry */
    __m128i c = _mm_clmulepi64_si128(a, b, 0x00);
    if (c.uq[0] == 0) {
        printf("PCLMULQDQ failed\n");
        return 1;
    }
    printf("AES / PCLMUL [PASS]\n");
    return 0;
}

int test_avx512(void) {
    __m512 a = _mm512_set1_ps(10.0f);
    __m512 b = _mm512_set1_ps(20.0f);
    __m512 c = _mm512_add_ps(a, b);
    float res[16];
    _mm512_storeu_ps(res, c);
    for (int i = 0; i < 16; i++) {
        if (res[i] != 30.0f) {
            printf("AVX-512 add_ps failed at %d: got %f\n", i, res[i]);
            return 1;
        }
    }
    printf("AVX-512 [PASS]\n");
    return 0;
}

int main(void) {
    printf("--- Running SIMD Intrinsics Verification Tests ---\n");
    if (test_sse()) return 1;
    if (test_sse2()) return 1;
    if (test_sse3()) return 1;
    if (test_ssse3()) return 1;
    if (test_sse4()) return 1;
    if (test_avx()) return 1;
    if (test_avx2()) return 1;
    if (test_fma()) return 1;
    if (test_aes_clmul()) return 1;
    if (test_avx512()) return 1;
    printf("--- ALL INTRINSICS TESTS PASSED SUCCESSFULLY! ---\n");
    return 0;
}
