#include <stdio.h>

int main() {
    double pi = 3.14159;
    int int_pi = static_cast<int>(pi);
    printf("static_cast: %d\n", int_pi);

    int num = 0x12345678;
    void *vptr = reinterpret_cast<void*>((long)num);
    long back = reinterpret_cast<long>(vptr);
    printf("reinterpret_cast: 0x%lx\n", back);

    const int cval = 100;
    const int *cptr = &cval;
    int *ncptr = const_cast<int*>(cptr);
    printf("const_cast: %d\n", *ncptr);

    void *dptr = dynamic_cast<void*>(ncptr);
    if (dptr != NULL) {
        printf("dynamic_cast: non-null\n");
    }

    return 0;
}
