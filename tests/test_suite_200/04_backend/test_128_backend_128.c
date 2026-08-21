#include <stdio.h>
#include <assert.h>
int main(void) {
    int val = 128 * 10;
    assert(val == 1280);
    printf("TEST_128_BACKEND_128_OK\n");
    return 0;
}
