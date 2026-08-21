#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

static const int16_t step_table[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static const int index_table[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

static uint8_t adpcm_encode_sample(int16_t sample, int16_t *pred, int *index) {
    int step = step_table[*index];
    int diff = sample - *pred;
    uint8_t code = 0;
    if (diff < 0) { code = 8; diff = -diff; }
    int mask = 4;
    int tempstep = step;
    int diffq = step >> 3;
    for (int i = 0; i < 3; i++) {
        if (diff >= tempstep) {
            code |= mask;
            diff -= tempstep;
            diffq += tempstep;
        }
        tempstep >>= 1;
        mask >>= 1;
    }
    if (code & 8) *pred -= diffq;
    else *pred += diffq;
    *index += index_table[code];
    if (*index < 0) *index = 0;
    if (*index > 88) *index = 88;
    return code;
}

int main(void) {
    int N = 200000;
    int16_t *pcm = (int16_t*)malloc(N * sizeof(int16_t));
    for (int i = 0; i < N; i++) pcm[i] = (int16_t)((i * 181) % 32000 - 16000);
    
    uint64_t checksum = 0;
    for (int iter = 0; iter < 20; iter++) {
        int16_t pred = 0;
        int idx = 0;
        for (int i = 0; i < N; i++) {
            uint8_t c = adpcm_encode_sample(pcm[i], &pred, &idx);
            checksum += c;
        }
    }
    printf("AudioADPCM: Checksum=%llu\n", (unsigned long long)checksum);
    free(pcm);
    return 0;
}