#include <stdio.h>

#define WIDTH 1200
#define HEIGHT 1200
#define MAX_ITER 100

int main(void) {
    int total_escaped = 0;
    long long iter_sum = 0;

    for (int y = 0; y < HEIGHT; y++) {
        double ci = (double)y / (double)HEIGHT * 2.0 - 1.0;
        for (int x = 0; x < WIDTH; x++) {
            double cr = (double)x / (double)WIDTH * 3.0 - 2.0;
            double zr = 0.0, zi = 0.0;
            int iter = 0;

            while (zr * zr + zi * zi <= 4.0 && iter < MAX_ITER) {
                double temp = zr * zr - zi * zi + cr;
                zi = 2.0 * zr * zi + ci;
                zr = temp;
                iter++;
            }

            if (iter < MAX_ITER) {
                total_escaped++;
            }
            iter_sum += iter;
        }
    }

    printf("Mandelbrot: W=%d H=%d Escaped=%d Sum=%lld\n", WIDTH, HEIGHT, total_escaped, iter_sum);
    return 0;
}
