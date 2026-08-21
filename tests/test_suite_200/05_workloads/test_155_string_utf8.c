#include <stdio.h>
#include <stdlib.h>

int main(void) {
    long long checksum = 0;
    for (int i = 0; i < 50000; i++) {
        checksum += (i * 155) ^ (i >> 3);
    }
    printf("String_utf8: Checksum=%lld\n", checksum);
    return 0;
}
