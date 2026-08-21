#include <stdio.h>
#include <stdint.h>

#define LIMBS 16 // 512-bit integer

typedef struct {
    uint32_t d[LIMBS];
} BigInt;

static void bigint_mul(BigInt *res, const BigInt *a, const BigInt *b) {
    uint64_t t[LIMBS * 2] = {0};
    for (int i = 0; i < LIMBS; i++) {
        for (int j = 0; j < LIMBS; j++) {
            t[i + j] += (uint64_t)a->d[i] * b->d[j];
            t[i + j + 1] += t[i + j] >> 32;
            t[i + j] &= 0xFFFFFFFF;
        }
    }
    for (int i = 0; i < LIMBS; i++) res->d[i] = (uint32_t)t[i];
}

int main(void) {
    BigInt base, exp_res;
    for (int i = 0; i < LIMBS; i++) {
        base.d[i] = (uint32_t)(i * 12345 + 6789);
        exp_res.d[i] = (i == 0) ? 1 : 0;
    }
    for (int iter = 0; iter < 10000; iter++) {
        bigint_mul(&exp_res, &exp_res, &base);
    }
    uint64_t checksum = 0;
    for (int i = 0; i < LIMBS; i++) checksum += exp_res.d[i];
    printf("CryptoRSABigInt: Iters=10000 Checksum=%llu\n", (unsigned long long)checksum);
    return 0;
}