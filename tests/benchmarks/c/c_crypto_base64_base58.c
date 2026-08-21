#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void base64_encode(char *dst, const uint8_t *src, size_t len) {
    size_t di = 0;
    for (size_t i = 0; i < len; i += 3) {
        uint32_t v = (src[i] << 16) | (src[i+1] << 8) | src[i+2];
        dst[di++] = b64_table[(v >> 18) & 0x3F];
        dst[di++] = b64_table[(v >> 12) & 0x3F];
        dst[di++] = b64_table[(v >> 6) & 0x3F];
        dst[di++] = b64_table[v & 0x3F];
    }
    dst[di] = 0;
}

int main(void) {
    size_t raw_len = 3000;
    uint8_t *raw = (uint8_t*)malloc(raw_len);
    for (size_t i = 0; i < raw_len; i++) raw[i] = (uint8_t)(i * 37 + 11);
    char *encoded = (char*)malloc(raw_len * 2);
    
    uint64_t checksum = 0;
    for (int iter = 0; iter < 10000; iter++) {
        raw[0] = (uint8_t)iter;
        base64_encode(encoded, raw, raw_len);
        checksum += encoded[0] + encoded[100] + encoded[1000];
    }
    printf("CryptoBase64: Iters=10000 Checksum=%llu\n", (unsigned long long)checksum);
    free(raw);
    free(encoded);
    return 0;
}
