#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DICT 4096

int main(void) {
    int N = 20000;
    uint8_t *data = (uint8_t*)malloc(N);
    for (int i = 0; i < N; i++) data[i] = (uint8_t)(i % 16);

    uint64_t checksum = 0;
    for (int iter = 0; iter < 100; iter++) {
        int dict_size = 256;
        int last_code = data[0];
        for (int i = 1; i < N; i++) {
            uint8_t next_char = data[i];
            if (dict_size < MAX_DICT) {
                dict_size++;
                last_code = next_char;
            } else {
                dict_size = 256;
            }
            checksum += last_code;
        }
    }
    printf("CompressLZW: Checksum=%llu\n", (unsigned long long)checksum);
    free(data);
    return 0;
}