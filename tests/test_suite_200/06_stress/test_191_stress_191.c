#include <stdio.h>
#include <assert.h>
int main(void) {
    int val = 191 * 10;
    assert(val == 1910);
    printf("TEST_191_STRESS_191_OK\n");
    return 0;
}
