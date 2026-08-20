#include <stdio.h>

#ifndef __cplusplus
#error "__cplusplus must be defined in C++ mode"
#endif

int main() {
    printf("__cplusplus is defined\n");

    bool b1 = true;
    bool b2 = false;

    if (b1) {
        printf("b1 is true\n");
    }
    if (!b2) {
        printf("b2 is false\n");
    }

    void *ptr = nullptr;
    if (ptr == NULL) {
        printf("nullptr is null\n");
    }

    printf("sizeof(bool) = %d\n", (int)sizeof(bool));
    return 0;
}
