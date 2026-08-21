#include <stdio.h>
#include <stdlib.h>

int main(void) {
    long long checksum = 0;
    for (int i = 0; i < 50000; i++) {
        checksum += (i * 160) ^ (i >> 3);
    }
    printf("Mapreduce_stream: Checksum=%lld\n", checksum);
    return 0;
}
