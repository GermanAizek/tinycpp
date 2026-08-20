#include <stdio.h>

struct Base {
    int a;
    int b;
};

class Derived : public Base {
public:
    int c;
};

class MultiLevel : public Derived {
public:
    int d;
};

int main() {
    Derived d;
    d.a = 10;
    d.b = 20;
    d.c = 30;

    printf("Derived: a=%d, b=%d, c=%d, sizeof=%d\n", d.a, d.b, d.c, (int)sizeof(Derived));

    MultiLevel m;
    m.a = 1;
    m.b = 2;
    m.c = 3;
    m.d = 4;
    printf("MultiLevel: a=%d, b=%d, c=%d, d=%d, sizeof=%d\n", m.a, m.b, m.c, m.d, (int)sizeof(MultiLevel));
    return 0;
}
