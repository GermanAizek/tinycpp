#include <stdio.h>

extern "C" {
    int add(int a, int b) {
        return a + b;
    }

    int mul(int a, int b) {
        return a * b;
    }
}

extern "C" int sub(int a, int b) {
    return a - b;
}

extern "C++" {
    int divide(int a, int b) {
        return a / b;
    }
}

int main() {
    printf("add(3, 4) = %d\n", add(3, 4));
    printf("mul(3, 4) = %d\n", mul(3, 4));
    printf("sub(10, 4) = %d\n", sub(10, 4));
    printf("divide(20, 4) = %d\n", divide(20, 4));
    return 0;
}
