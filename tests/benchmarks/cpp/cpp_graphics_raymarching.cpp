#include <stdio.h>
#include <math.h>

static float sphere_sdf(float x, float y, float z, float r) {
    return sqrtf(x*x + y*y + z*z) - r;
}

static float scene_sdf(float x, float y, float z) {
    float d1 = sphere_sdf(x, y, z - 3.0f, 1.0f);
    float d2 = sphere_sdf(x - 0.8f, y - 0.5f, z - 2.5f, 0.5f);
    return fminf(d1, d2);
}

int main(void) {
    int hit_count = 0;
    float depth_sum = 0.0f;
    for (int y = -100; y < 100; y++) {
        for (int x = -100; x < 100; x++) {
            float rx = x * 0.01f;
            float ry = y * 0.01f;
            float rz = 1.0f;
            float len = sqrtf(rx*rx + ry*ry + rz*rz);
            rx /= len; ry /= len; rz /= len;
            
            float t = 0.0f;
            for (int step = 0; step < 64; step++) {
                float px = rx * t;
                float py = ry * t;
                float pz = rz * t;
                float dist = scene_sdf(px, py, pz);
                if (dist < 0.001f) {
                    hit_count++;
                    depth_sum += t;
                    break;
                }
                t += dist;
                if (t > 10.0f) break;
            }
        }
    }
    printf("GraphicsRaymarching: Hits=%d DepthSum=%.2f\n", hit_count, depth_sum);
    return 0;
}