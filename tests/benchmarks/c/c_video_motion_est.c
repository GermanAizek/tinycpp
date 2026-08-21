#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define W 128
#define H 128
#define BLOCK 16
#define SEARCH 8

static uint8_t curr_frame[H][W];
static uint8_t ref_frame[H][W];

static int compute_sad(int cx, int cy, int rx, int ry) {
    int sad = 0;
    for (int y = 0; y < BLOCK; y++) {
        for (int x = 0; x < BLOCK; x++) {
            sad += abs(curr_frame[cy + y][cx + x] - ref_frame[ry + y][rx + x]);
        }
    }
    return sad;
}

int main(void) {
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            curr_frame[y][x] = (uint8_t)(x * 3 + y * 7);
            ref_frame[y][x] = (uint8_t)((x + 2) * 3 + (y + 1) * 7);
        }
    }
    int total_sad = 0;
    for (int by = 0; by <= H - BLOCK; by += BLOCK) {
        for (int bx = 0; bx <= W - BLOCK; bx += BLOCK) {
            int min_sad = 1000000;
            for (int dy = -SEARCH; dy <= SEARCH; dy++) {
                for (int dx = -SEARCH; dx <= SEARCH; dx++) {
                    int rx = bx + dx, ry = by + dy;
                    if (rx >= 0 && rx <= W - BLOCK && ry >= 0 && ry <= H - BLOCK) {
                        int sad = compute_sad(bx, by, rx, ry);
                        if (sad < min_sad) min_sad = sad;
                    }
                }
            }
            total_sad += min_sad;
        }
    }
    printf("VideoMotionEst: TotalSAD=%d\n", total_sad);
    return 0;
}