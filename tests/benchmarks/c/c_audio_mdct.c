#include <stdio.h>
#include <math.h>

#define N 512
#define PI 3.14159265358979323846

static float in_audio[N];
static float out_mdct[N / 2];

static void compute_mdct(void) {
    for (int k = 0; k < N / 2; k++) {
        float sum = 0.0f;
        for (int n = 0; n < N; n++) {
            float window = sinf((PI / N) * (n + 0.5f));
            float angle = (PI / (N / 2)) * (n + 0.5f + (N / 4.0f)) * (k + 0.5f);
            sum += in_audio[n] * window * cosf(angle);
        }
        out_mdct[k] = sum;
    }
}

int main(void) {
    for (int i = 0; i < N; i++) in_audio[i] = sinf(2.0f * PI * 440.0f * i / 44100.0f);
    float checksum = 0.0f;
    for (int iter = 0; iter < 200; iter++) {
        in_audio[0] = (float)(iter % 10);
        compute_mdct();
        checksum += out_mdct[0] + out_mdct[N/4] + out_mdct[N/2 - 1];
    }
    printf("AudioMDCT: Checksum=%.4f\n", checksum);
    return 0;
}
