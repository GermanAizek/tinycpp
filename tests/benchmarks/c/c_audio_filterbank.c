#include <stdio.h>
#include <math.h>

#define CHANNELS 32
#define SAMPLES 1000

static float subbands[CHANNELS][SAMPLES];

int main(void) {
    float checksum = 0.0f;
    for (int t = 0; t < SAMPLES; t++) {
        for (int ch = 0; ch < CHANNELS; ch++) {
            float phase = (2.0f * 3.14159265f * (ch + 0.5f) * t) / CHANNELS;
            subbands[ch][t] = cosf(phase);
            checksum += subbands[ch][t] * (ch + 1);
        }
    }
    printf("AudioFilterbank: Checksum=%.2f\n", checksum);
    return 0;
}