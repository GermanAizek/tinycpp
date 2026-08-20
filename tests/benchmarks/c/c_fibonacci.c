#include <stdio.h>

static long long fib_recursive(int n) {
    if (n <= 1) return n;
    return fib_recursive(n - 1) + fib_recursive(n - 2);
}

static long long fib_iterative(int n) {
    if (n <= 1) return n;
    long long a = 0, b = 1, c = 0;
    for (int i = 2; i <= n; i++) {
        c = a + b;
        a = b;
        b = c;
    }
    return b;
}

int main(void) {
    long long sum = 0;

    /* Recursive Fib(38) */
    long long r1 = fib_recursive(38);
    sum += r1;

    /* Many iterative fibs */
    for (int i = 0; i < 1000000; i++) {
        sum += (fib_iterative(i % 50) % 100);
    }

    printf("Fibonacci: Fib(38)=%lld Checksum=%lld\n", r1, sum);
    return 0;
}
