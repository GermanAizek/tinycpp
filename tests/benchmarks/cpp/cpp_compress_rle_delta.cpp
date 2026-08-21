#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main(void) {
    int N = 100000;
    uint8_t *raw = (uint8_t*)malloc(N);
    uint8_t *delta = (uint8_t*)malloc(N);
    uint8_t *rle_out = (uint8_t*)malloc(N * 2);

    for (int i = 0; i < N; i++) raw[i] = (uint8_t)(i / 10);

    uint64_t checksum = 0;
    for (int iter = 0; iter < 100; iter++) {
        // Delta encoding
        delta[0] = raw[0];
        for (int i = 1; i < N; i++) delta[i] = raw[i] - raw[i - 1];

        // RLE encoding
        int out_idx = 0;
        int run_len = 1;
        for (int i = 1; i < N; i++) {
            if (delta[i] == delta[i - 1] && run_len < 255) {
                run_len++;
            } else {
                rle_out[out_idx++] = (uint8_t)run_len;
                rle_out[out_idx++] = delta[i - 1];
                run_len = 1;
            }
        }
        rle_out[out_idx++] = (uint8_t)run_len;
        rle_out[out_idx++] = delta[N - 1];
        checksum += out_idx;
    }
    printf("CompressRLEDelta: Checksum=%llu\n", (unsigned long long)checksum);
    free(raw);
    free(delta);
    free(rle_out);
    return 0;
}