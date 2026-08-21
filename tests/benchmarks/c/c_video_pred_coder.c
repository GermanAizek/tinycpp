#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 4

static void intra_pred_4x4(const uint8_t *top, const uint8_t *left, uint8_t dst_vert[N][N], uint8_t dst_horiz[N][N], uint8_t dst_dc[N][N]) {
    int dc = 0;
    for (int i = 0; i < N; i++) dc += top[i] + left[i];
    dc = (dc + 4) >> 3;

    for (int y = 0; y < N; y++) {
        for (int x = 0; x < N; x++) {
            dst_vert[y][x] = top[x];
            dst_horiz[y][x] = left[y];
            dst_dc[y][x] = (uint8_t)dc;
        }
    }
}

int main(void) {
    uint8_t top[4] = {10, 20, 30, 40};
    uint8_t left[4] = {50, 60, 70, 80};
    uint8_t v[N][N], h[N][N], d[N][N];
    
    uint64_t checksum = 0;
    for (int iter = 0; iter < 500000; iter++) {
        top[0] = (uint8_t)iter;
        intra_pred_4x4(top, left, v, h, d);
        checksum += v[0][0] + h[1][1] + d[3][3];
    }
    printf("VideoPredCoder: Checksum=%llu\n", (unsigned long long)checksum);
    return 0;
}