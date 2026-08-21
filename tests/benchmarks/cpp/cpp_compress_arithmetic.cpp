#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define TOP_VALUE 0xFFFFFFFFU
#define FIRST_QTR (TOP_VALUE / 4 + 1)
#define HALF      (2 * FIRST_QTR)
#define THIRD_QTR (3 * FIRST_QTR)

int main(void) {
    int N = 50000;
    uint8_t *symbols = (uint8_t*)malloc(N);
    for (int i = 0; i < N; i++) symbols[i] = (uint8_t)(i % 4);

    uint64_t checksum = 0;
    for (int iter = 0; iter < 50; iter++) {
        uint32_t low = 0;
        uint32_t high = TOP_VALUE;
        for (int i = 0; i < N; i++) {
            uint32_t range = high - low + 1;
            uint8_t sym = symbols[i];
            high = low + (range * (sym + 1)) / 4 - 1;
            low  = low + (range * sym) / 4;

            while (high < HALF || low >= HALF) {
                if (high < HALF) {
                    low <<= 1;
                    high = (high << 1) + 1;
                    checksum++;
                } else if (low >= HALF) {
                    low = (low - HALF) << 1;
                    high = ((high - HALF) << 1) + 1;
                    checksum += 2;
                }
            }
        }
    }
    printf("CompressArithmetic: Checksum=%llu\n", (unsigned long long)checksum);
    free(symbols);
    return 0;
}