#include <stdio.h>

#define WIDTH 1200
#define HEIGHT 1200
#define MAX_ITER 100

struct Complex {
    double r;
    double i;
};

double complex_norm_sq(const Complex &c) {
    return c.r * c.r + c.i * c.i;
}

void complex_square_add(Complex &z, const Complex &c) {
    double new_r = z.r * z.r - z.i * z.i + c.r;
    double new_i = 2.0 * z.r * z.i + c.i;
    z.r = new_r;
    z.i = new_i;
}

int main() {
    int total_escaped = 0;
    long long iter_sum = 0;

    for (int y = 0; y < HEIGHT; y++) {
        Complex c;
        c.i = (double)y / (double)HEIGHT * 2.0 - 1.0;

        for (int x = 0; x < WIDTH; x++) {
            c.r = (double)x / (double)WIDTH * 3.0 - 2.0;

            Complex z;
            z.r = 0.0;
            z.i = 0.0;
            int iter = 0;

            while (complex_norm_sq(z) <= 4.0 && iter < MAX_ITER) {
                complex_square_add(z, c);
                iter++;
            }

            if (iter < MAX_ITER) {
                total_escaped++;
            }
            iter_sum += iter;
        }
    }

    printf("CppMandelbrot: W=%d H=%d Escaped=%d Sum=%lld\n", WIDTH, HEIGHT, total_escaped, iter_sum);
    return 0;
}
