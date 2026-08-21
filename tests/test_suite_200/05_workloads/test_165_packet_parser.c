#include <stdio.h>
#include <stdlib.h>

int main(void) {
    long long checksum = 0;
    for (int i = 0; i < 50000; i++) {
        checksum += (i * 165) ^ (i >> 3);
    }
    printf("Packet_parser: Checksum=%lld\n", checksum);
    return 0;
}
