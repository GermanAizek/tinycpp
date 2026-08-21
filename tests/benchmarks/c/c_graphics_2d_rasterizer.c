#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define W 512
#define H 512
static uint32_t fb[W * H];

static void bresenham_line(int x0, int y0, int x1, int y1, uint32_t col) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (1) {
        if (x0 >= 0 && x0 < W && y0 >= 0 && y0 < H) fb[y0 * W + x0] ^= col;
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

int main(void) {
    for (int i = 0; i < 5000; i++) {
        int x0 = (i * 37) % W, y0 = (i * 73) % H;
        int x1 = (i * 101) % W, y1 = (i * 137) % H;
        bresenham_line(x0, y0, x1, y1, (uint32_t)(i * 0x1234567));
    }
    uint64_t checksum = 0;
    for (int i = 0; i < W * H; i++) checksum += fb[i];
    printf("Graphics2DRasterizer: Checksum=%llu\n", (unsigned long long)checksum);
    return 0;
}