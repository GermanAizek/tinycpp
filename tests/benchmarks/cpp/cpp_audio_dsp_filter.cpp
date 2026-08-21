#include <stdio.h>
#include <math.h>

#define SAMPLES 100000

static float signal[SAMPLES];
static float filtered[SAMPLES];

static void biquad_iir(float *data, int n, float b0, float b1, float b2, float a1, float a2) {
    float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    for (int i = 0; i < n; i++) {
        float x0 = data[i];
        float y0 = b0*x0 + b1*x1 + b2*x2 - a1*y1 - a2*y2;
        x2 = x1; x1 = x0;
        y2 = y1; y1 = y0;
        data[i] = y0;
    }
}

int main(void) {
    for (int i = 0; i < SAMPLES; i++) signal[i] = sinf(0.1f * i) + 0.5f * sinf(0.8f * i);
    for (int iter = 0; iter < 50; iter++) {
        biquad_iir(signal, SAMPLES, 0.2929f, 0.5858f, 0.2929f, -0.0f, 0.1716f);
    }
    float checksum = 0.0f;
    for (int i = 0; i < SAMPLES; i += 100) checksum += signal[i];
    printf("AudioDSPFilter: Checksum=%.4f\n", checksum);
    return 0;
}