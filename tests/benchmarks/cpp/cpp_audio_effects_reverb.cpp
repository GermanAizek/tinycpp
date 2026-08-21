#include <stdio.h>
#include <stdlib.h>

#define DELAY_LEN 1024
#define SAMPLES 100000

static float delay_buf[DELAY_LEN];
static int d_idx = 0;

static float comb_filter(float input, float feedback) {
    float delayed = delay_buf[d_idx];
    float out = delayed;
    delay_buf[d_idx] = input + delayed * feedback;
    d_idx = (d_idx + 1) % DELAY_LEN;
    return out;
}

int main(void) {
    float checksum = 0.0f;
    for (int i = 0; i < SAMPLES; i++) {
        float in = (i % 100 == 0) ? 1.0f : 0.0f;
        float out = comb_filter(in, 0.85f);
        checksum += out;
    }
    printf("AudioEffectsReverb: Checksum=%.4f\n", checksum);
    return 0;
}