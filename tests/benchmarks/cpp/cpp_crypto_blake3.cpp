#include <stdio.h>
#include <stdint.h>

static const uint32_t IV[8] = {
    0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
    0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
};

#define ROTR32(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define G(v, a, b, c, d, x, y) do { \
    v[a] += v[b] + (x); v[d] = ROTR32(v[d] ^ v[a], 16); \
    v[c] += v[d];       v[b] = ROTR32(v[b] ^ v[c], 12); \
    v[a] += v[b] + (y); v[d] = ROTR32(v[d] ^ v[a], 8);  \
    v[c] += v[d];       v[b] = ROTR32(v[b] ^ v[c], 7);  \
} while(0)

static void blake3_compress(uint32_t cv[8], const uint32_t block[16]) {
    uint32_t v[16];
    for (int i = 0; i < 8; i++) v[i] = cv[i];
    for (int i = 0; i < 8; i++) v[i+8] = IV[i];
    for (int r = 0; r < 7; r++) {
        G(v, 0, 4, 8, 12, block[0], block[1]);
        G(v, 1, 5, 9, 13, block[2], block[3]);
        G(v, 2, 6, 10, 14, block[4], block[5]);
        G(v, 3, 7, 11, 15, block[6], block[7]);
        G(v, 0, 5, 10, 15, block[8], block[9]);
        G(v, 1, 6, 11, 12, block[10], block[11]);
        G(v, 2, 7, 8, 13, block[12], block[13]);
        G(v, 3, 4, 9, 14, block[14], block[15]);
    }
    for (int i = 0; i < 8; i++) cv[i] = v[i] ^ v[i+8];
}

int main(void) {
    uint32_t cv[8];
    uint32_t block[16];
    for (int i = 0; i < 8; i++) cv[i] = IV[i];
    for (int i = 0; i < 16; i++) block[i] = (uint32_t)(i * 1234567);
    
    uint64_t checksum = 0;
    for (int iter = 0; iter < 50000; iter++) {
        block[0] = (uint32_t)iter;
        blake3_compress(cv, block);
        checksum += cv[0] ^ cv[7];
    }
    printf("CryptoBLAKE3: Iters=50000 Checksum=%llu\n", (unsigned long long)checksum);
    return 0;
}