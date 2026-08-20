#include <stdio.h>

void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void increment(int &val) {
    int *p = &val;
    *p = *p + 1;
}

int main() {
    int x = 42;
    int &ref = x;
    int *ptr = &ref;

    printf("x = %d, ref = %d\n", x, *ptr);
    *ptr = 100;
    printf("after modify: x = %d\n", x);

    int a = 5, b = 10;
    swap(&a, &b);
    printf("swap: a=%d, b=%d\n", a, b);

    increment(a);
    printf("increment: a=%d\n", a);
    return 0;
}
