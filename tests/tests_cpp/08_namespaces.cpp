#include <stdio.h>

int global_var = 100;

namespace Math {
    int add(int a, int b) {
        return a + b;
    }
    int sub(int a, int b) {
        return a - b;
    }
}

namespace Outer {
    namespace Inner {
        int get_secret() {
            return 42;
        }
    }
}

using namespace Math;

int main() {
    printf("add: %d\n", add(10, 20));
    printf("sub: %d\n", sub(50, 15));
    printf("Outer::Inner::get_secret(): %d\n", Outer::Inner::get_secret());
    printf("::global_var: %d\n", ::global_var);
    return 0;
}
