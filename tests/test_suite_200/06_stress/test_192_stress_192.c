#include <stdio.h>
#include <assert.h>
int main(void) {
    int val = 192 * 10;
    assert(val == 1920);
    printf("TEST_192_STRESS_192_OK\n");
    return 0;
}
