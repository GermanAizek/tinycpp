#include <stdio.h>
#include <assert.h>
int main(void) {
    int val = 43 * 10;
    assert(val == 430);
    printf("TEST_043_SEMANTICS_43_OK\n");
    return 0;
}
