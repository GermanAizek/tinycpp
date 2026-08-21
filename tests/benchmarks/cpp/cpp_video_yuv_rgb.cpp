#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define W 256
#define H 256

static uint8_t y_plane[W * H];
static uint8_t u_plane[W * H / 4];
static uint8_t v_plane[W * H / 4];
static uint8_t rgb_out[W * H * 3];

static inline uint8_t clamp8(int v) {
    return v < 0 ? 0 : (v > 255 ? 255 : (uint8_t)v);
}

static void yuv420_to_rgb(void) {
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int Y = y_plane[y * W + x];
            int U = u_plane[(y/2) * (W/2) + (x/2)] - 128;
            int V = v_plane[(y/2) * (W/2) + (x/2)] - 128;

            int R = Y + ((1436 * V) >> 10);
            int G = Y - ((352 * U + 731 * V) >> 10);
            int B = Y + ((1815 * U) >> 10);

            int idx = (y * W + x) * 3;
            rgb_out[idx + 0] = clamp8(R);
            rgb_out[idx + 1] = clamp8(G);
            rgb_out[idx + 2] = clamp8(B);
        }
    }
}

int main(void) {
    for (int i = 0; i < W * H; i++) y_plane[i] = (uint8_t)(i & 0xFF);
    for (int i = 0; i < W * H / 4; i++) {
        u_plane[i] = (uint8_t)((i * 3) & 0xFF);
        v_plane[i] = (uint8_t)((i * 7) & 0xFF);
    }
    for (int iter = 0; iter < 100; iter++) {
        yuv420_to_rgb();
        y_plane[0] = (uint8_t)iter;
    }
    uint64_t checksum = 0;
    for (int i = 0; i < W * H * 3; i++) checksum += rgb_out[i];
    printf("VideoYUV_RGB: Checksum=%llu\n", (unsigned long long)checksum);
    return 0;
}