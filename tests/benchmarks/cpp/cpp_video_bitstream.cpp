#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    const uint8_t *buf;
    size_t bit_pos;
    size_t total_bits;
} BitReader;

static inline uint32_t read_bit(BitReader *br) {
    if (br->bit_pos >= br->total_bits) return 0;
    uint32_t b = (br->buf[br->bit_pos >> 3] >> (7 - (br->bit_pos & 7))) & 1;
    br->bit_pos++;
    return b;
}

static uint32_t read_exp_golomb(BitReader *br) {
    int leading_zeros = 0;
    while (read_bit(br) == 0 && leading_zeros < 31) leading_zeros++;
    uint32_t code_num = 1;
    for (int i = 0; i < leading_zeros; i++) {
        code_num = (code_num << 1) | read_bit(br);
    }
    return code_num - 1;
}

int main(void) {
    uint8_t stream[1024];
    for (int i = 0; i < 1024; i++) stream[i] = (uint8_t)(i * 13 + 7);
    
    uint64_t checksum = 0;
    for (int iter = 0; iter < 10000; iter++) {
        BitReader br = {stream, 0, 1024 * 8};
        while (br.bit_pos < 1024 * 8 - 32) {
            checksum += read_exp_golomb(&br);
        }
    }
    printf("VideoBitstream: Checksum=%llu\n", (unsigned long long)checksum);
    return 0;
}