#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 256
static uint8_t src[N][N];
static uint8_t dst[N][N];

static void gaussian_5x5_sobel(void) {
    for (int y = 2; y < N - 2; y++) {
        for (int x = 2; x < N - 2; x++) {
            int gx = -src[y-1][x-1] + src[y-1][x+1] - 2*src[y][x-1] + 2*src[y][x+1] - src[y+1][x-1] + src[y+1][x+1];
            int gy = -src[y-1][x-1] - 2*src[y-1][x] - src[y-1][x+1] + src[y+1][x-1] + 2*src[y+1][x] + src[y+1][x+1];
            int mag = abs(gx) + abs(gy);
            dst[y][x] = mag > 255 ? 255 : (uint8_t)mag;
        }
    }
}

int main(void) {
    for (int y = 0; y < N; y++) {
        for (int x = 0; x < N; x++) src[y][x] = (uint8_t)((x ^ y) + (x * y));
    }
    for (int iter = 0; iter < 100; iter++) {
        gaussian_5x5_sobel();
        src[128][128] = (uint8_t)iter;
    }
    uint64_t checksum = 0;
    for (int y = 0; y < N; y++) {
        for (int x = 0; x < N; x++) checksum += dst[y][x];
    }
    printf("GraphicsImageFilter: Checksum=%llu\n", (unsigned long long)checksum);
    return 0;
}