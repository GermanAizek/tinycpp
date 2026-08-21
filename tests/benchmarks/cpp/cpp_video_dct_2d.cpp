#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846

static float block[8][8];
static float dct_out[8][8];

static void fdct_8x8(void) {
    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            float cu = (u == 0) ? 1.0f / sqrtf(2.0f) : 1.0f;
            float cv = (v == 0) ? 1.0f / sqrtf(2.0f) : 1.0f;
            float sum = 0.0f;
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    sum += block[x][y] * cosf((2*x + 1)*u*PI/16.0f) * cosf((2*y + 1)*v*PI/16.0f);
                }
            }
            dct_out[u][v] = 0.25f * cu * cv * sum;
        }
    }
}

int main(void) {
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) block[x][y] = (float)(x * 10 + y * 20);
    }
    float checksum = 0.0f;
    for (int iter = 0; iter < 2000; iter++) {
        block[0][0] = (float)(iter % 255);
        fdct_8x8();
        checksum += dct_out[0][0] + dct_out[1][1] + dct_out[7][7];
    }
    printf("VideoDCT2D: Checksum=%.2f\n", checksum);
    return 0;
}
