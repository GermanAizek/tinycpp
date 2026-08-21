#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 256
#define HEIGHT 256

typedef struct { float x, y, z; } Vec3;

static float zbuffer[WIDTH * HEIGHT];
static uint32_t framebuffer[WIDTH * HEIGHT];

static void draw_triangle(Vec3 v0, Vec3 v1, Vec3 v2, uint32_t color) {
    int min_x = (int)fmaxf(0, fminf(v0.x, fminf(v1.x, v2.x)));
    int max_x = (int)fminf(WIDTH - 1, fmaxf(v0.x, fmaxf(v1.x, v2.x)));
    int min_y = (int)fmaxf(0, fminf(v0.y, fminf(v1.y, v2.y)));
    int max_y = (int)fminf(HEIGHT - 1, fmaxf(v0.y, fmaxf(v1.y, v2.y)));
    
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            float w0 = (v1.y - v2.y)*(x - v2.x) + (v2.x - v1.x)*(y - v2.y);
            float w1 = (v2.y - v0.y)*(x - v2.x) + (v0.x - v2.x)*(y - v2.y);
            float denom = (v1.y - v2.y)*(v0.x - v2.x) + (v2.x - v1.x)*(v0.y - v2.y);
            if (denom != 0.0f) {
                w0 /= denom;
                w1 /= denom;
                float w2 = 1.0f - w0 - w1;
                if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                    float z = w0 * v0.z + w1 * v1.z + w2 * v2.z;
                    int idx = y * WIDTH + x;
                    if (z < zbuffer[idx]) {
                        zbuffer[idx] = z;
                        framebuffer[idx] = color;
                    }
                }
            }
        }
    }
}

int main(void) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        zbuffer[i] = 1000.0f;
        framebuffer[i] = 0;
    }
    for (int t = 0; t < 2000; t++) {
        float angle = t * 0.05f;
        Vec3 v0 = {128 + 80 * cosf(angle), 128 + 80 * sinf(angle), 1.0f + (t % 10)};
        Vec3 v1 = {128 + 80 * cosf(angle + 2.0f), 128 + 80 * sinf(angle + 2.0f), 2.0f + (t % 10)};
        Vec3 v2 = {128 + 80 * cosf(angle + 4.0f), 128 + 80 * sinf(angle + 4.0f), 3.0f + (t % 10)};
        draw_triangle(v0, v1, v2, 0xFF0000 | (t & 0xFF));
    }
    uint64_t checksum = 0;
    for (int i = 0; i < WIDTH * HEIGHT; i++) checksum += framebuffer[i];
    printf("Graphics3DRenderer: Checksum=%llu\n", (unsigned long long)checksum);
    return 0;
}