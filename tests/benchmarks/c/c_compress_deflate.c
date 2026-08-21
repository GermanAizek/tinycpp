#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define WINDOW 32768
#define HASH_MASK 8191

int main(void) {
    int N = 30000;
    uint8_t *buf = (uint8_t*)malloc(N);
    int head[HASH_MASK + 1];
    int prev[WINDOW];
    memset(head, -1, sizeof(head));

    for (int i = 0; i < N; i++) buf[i] = (uint8_t)((i * 7) % 64);

    uint64_t match_count = 0;
    for (int i = 0; i < N - 3; i++) {
        uint32_t h = ((buf[i] << 5) ^ (buf[i+1] << 2) ^ buf[i+2]) & HASH_MASK;
        int match = head[h];
        prev[i % WINDOW] = match;
        head[h] = i;
        if (match >= 0 && (i - match) < WINDOW) {
            match_count++;
        }
    }
    printf("CompressDeflate: Matches=%llu\n", (unsigned long long)match_count);
    free(buf);
    return 0;
}