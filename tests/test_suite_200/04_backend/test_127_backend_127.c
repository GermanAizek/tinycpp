#include <stdio.h>
#include <assert.h>
int main(void) {
    int val = 127 * 10;
    assert(val == 1270);
    printf("TEST_127_BACKEND_127_OK\n");
    return 0;
}
