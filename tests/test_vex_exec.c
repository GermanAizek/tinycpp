#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(void) {
    printf("Testing runtime inline assembly for AVX/AVX2/FMA/AES...\n");

    /* AVX 256-bit vaddps */
    float a[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    float b[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    float c[8] = {0};

    __asm__ __volatile__(
        "vmovups (%0), %%ymm0\n\t"
        "vmovups (%1), %%ymm1\n\t"
        "vaddps %%ymm1, %%ymm0, %%ymm2\n\t"
        "vmovups %%ymm2, (%2)\n\t"
        "vzeroupper\n\t"
        :
        : "r"(a), "r"(b), "r"(c)
        : "memory"
    );

    for (int i = 0; i < 8; i++) {
        if (c[i] != a[i] + b[i]) {
            printf("AVX vaddps failed at index %d: %f != %f\n", i, c[i], a[i] + b[i]);
            return 1;
        }
    }
    printf("AVX 256-bit vaddps & vzeroupper: PASS\n");

    /* AVX2 256-bit vpaddd */
    int32_t ia[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    int32_t ib[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int32_t ic[8] = {0};

    __asm__ __volatile__(
        "vmovdqu (%0), %%ymm3\n\t"
        "vmovdqu (%1), %%ymm4\n\t"
        "vpaddd %%ymm4, %%ymm3, %%ymm5\n\t"
        "vmovdqu %%ymm5, (%2)\n\t"
        "vzeroupper\n\t"
        :
        : "r"(ia), "r"(ib), "r"(ic)
        : "memory"
    );

    for (int i = 0; i < 8; i++) {
        if (ic[i] != ia[i] + ib[i]) {
            printf("AVX2 vpaddd failed at index %d: %d != %d\n", i, ic[i], ia[i] + ib[i]);
            return 1;
        }
    }
    printf("AVX2 256-bit vpaddd: PASS\n");

    /* FMA3 256-bit vfmadd213ps: ymm = ymm * ymm_src1 + ymm_src2 */
    /* ymm6 = ymm6 * ymm7 + ymm8 -> c = a * b + b */
    float fa[8] = {2, 2, 2, 2, 2, 2, 2, 2};
    float fb[8] = {3, 3, 3, 3, 3, 3, 3, 3};
    float fc[8] = {4, 4, 4, 4, 4, 4, 4, 4};
    float f_res[8] = {0};

    __asm__ __volatile__(
        "vmovups (%0), %%ymm6\n\t"
        "vmovups (%1), %%ymm7\n\t"
        "vmovups (%2), %%ymm8\n\t"
        "vfmadd213ps %%ymm8, %%ymm7, %%ymm6\n\t"
        "vmovups %%ymm6, (%3)\n\t"
        "vzeroupper\n\t"
        :
        : "r"(fa), "r"(fb), "r"(fc), "r"(f_res)
        : "memory"
    );

    /* fa * fb + fc = 2 * 3 + 4 = 10 */
    for (int i = 0; i < 8; i++) {
        if (f_res[i] != 10.0f) {
            printf("FMA3 vfmadd213ps failed at index %d: got %f, expected 10.0\n", i, f_res[i]);
            return 1;
        }
    }
    printf("FMA3 256-bit vfmadd213ps: PASS\n");

    /* AES-NI vaesenc */
    uint8_t aes_state[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    uint8_t round_key[16] = {0};
    uint8_t aes_out[16] = {0};

    __asm__ __volatile__(
        "vmovdqu (%0), %%xmm9\n\t"
        "vmovdqu (%1), %%xmm10\n\t"
        "vaesenc %%xmm10, %%xmm9, %%xmm11\n\t"
        "vmovdqu %%xmm11, (%2)\n\t"
        "vzeroupper\n\t"
        :
        : "r"(aes_state), "r"(round_key), "r"(aes_out)
        : "memory"
    );

    int aes_diff = 0;
    for (int i = 0; i < 16; i++) {
        if (aes_out[i] != aes_state[i]) aes_diff++;
    }
    if (aes_diff == 0) {
        printf("vaesenc failed to alter state!\n");
        return 1;
    }
    printf("AES-NI vaesenc: PASS\n");

    /* PCLMULQDQ vpclmulqdq */
    uint64_t pa[2] = {0x12345678ULL, 0x9abcdef0ULL};
    uint64_t pb[2] = {0x11111111ULL, 0x22222222ULL};
    uint64_t pres[2] = {0};

    __asm__ __volatile__(
        "vmovdqu (%0), %%xmm12\n\t"
        "vmovdqu (%1), %%xmm13\n\t"
        "vpclmulqdq $0x00, %%xmm13, %%xmm12, %%xmm14\n\t"
        "vmovdqu %%xmm14, (%2)\n\t"
        "vzeroupper\n\t"
        :
        : "r"(pa), "r"(pb), "r"(pres)
        : "memory"
    );

    if (pres[0] == 0) {
        printf("vpclmulqdq produced 0!\n");
        return 1;
    }
    printf("PCLMULQDQ vpclmulqdq: PASS\n");

    printf("\n=== ALL INLINE ASM SIMD HARDWARE EXECUTION TESTS PASSED! ===\n");
    return 0;
}
