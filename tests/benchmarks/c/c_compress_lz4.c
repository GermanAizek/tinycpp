#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 4096

static int lz4_compress_block(const uint8_t *src, int src_size, uint8_t *dst) {
    int hash_table[HASH_SIZE];
    memset(hash_table, -1, sizeof(hash_table));
    int src_idx = 0, dst_idx = 0;

    while (src_idx < src_size - 4) {
        uint32_t seq = *(uint32_t*)(src + src_idx);
        uint32_t h = (seq * 2654435761U) % HASH_SIZE;
        int match_pos = hash_table[h];
        hash_table[h] = src_idx;

        if (match_pos >= 0 && src_idx - match_pos < 65535 && *(uint32_t*)(src + match_pos) == seq) {
            dst[dst_idx++] = 0xF0; // match token
            dst[dst_idx++] = (uint8_t)(src_idx - match_pos);
            src_idx += 4;
        } else {
            dst[dst_idx++] = src[src_idx++];
        }
    }
    return dst_idx;
}

int main(void) {
    int N = 50000;
    uint8_t *raw = (uint8_t*)malloc(N);
    uint8_t *out = (uint8_t*)malloc(N * 2);
    for (int i = 0; i < N; i++) raw[i] = (uint8_t)((i % 20) + (i / 1000));

    uint64_t checksum = 0;
    for (int iter = 0; iter < 500; iter++) {
        raw[0] = (uint8_t)iter;
        int comp_size = lz4_compress_block(raw, N, out);
        checksum += comp_size;
    }
    printf("CompressLZ4: Checksum=%llu\n", (unsigned long long)checksum);
    free(raw);
    free(out);
    return 0;
}