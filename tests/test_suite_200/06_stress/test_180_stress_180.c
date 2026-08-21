#include <stdio.h>
#include <assert.h>
int main(void) {
    int val = 180 * 10;
    assert(val == 1800);
    printf("TEST_180_STRESS_180_OK\n");
    return 0;
}
