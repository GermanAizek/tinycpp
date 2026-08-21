#include <stdio.h>
#include <stdint.h>

static const uint64_t RC[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
    0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

#define ROL64(a, n) (((a) << (n)) | ((a) >> (64 - (n))))

static void keccak_f1600(uint64_t state[25]) {
    for (int round = 0; round < 24; round++) {
        // Theta
        uint64_t C[5], D[5];
        for (int i = 0; i < 5; i++) C[i] = state[i] ^ state[i+5] ^ state[i+10] ^ state[i+15] ^ state[i+20];
        for (int i = 0; i < 5; i++) D[i] = C[(i+4)%5] ^ ROL64(C[(i+1)%5], 1);
        for (int i = 0; i < 25; i++) state[i] ^= D[i%5];
        // Chi
        for (int j = 0; j < 25; j += 5) {
            uint64_t t[5];
            for (int i = 0; i < 5; i++) t[i] = state[j+i];
            for (int i = 0; i < 5; i++) state[j+i] ^= (~t[(i+1)%5]) & t[(i+2)%5];
        }
        state[0] ^= RC[round];
    }
}

int main(void) {
    uint64_t state[25] = {0};
    uint64_t checksum = 0;
    for (int i = 0; i < 50000; i++) {
        state[0] ^= (uint64_t)i * 0x9e3779b97f4a7c15ULL;
        keccak_f1600(state);
        checksum += state[0] ^ state[12] ^ state[24];
    }
    printf("CryptoSHA3: Iters=50000 Checksum=%llu\n", (unsigned long long)checksum);
    return 0;
}