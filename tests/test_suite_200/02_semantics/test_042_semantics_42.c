#include <stdio.h>
#include <assert.h>
int main(void) {
    int val = 42 * 10;
    assert(val == 420);
    printf("TEST_042_SEMANTICS_42_OK\n");
    return 0;
}
