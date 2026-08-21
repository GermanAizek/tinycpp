#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846

static void fft(double *real, double *imag, int n, int is_inverse) {
    int i, j, k, len;
    /* Bit-reversal permutation */
    for (i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            double tr = real[i]; real[i] = real[j]; real[j] = tr;
            double ti = imag[i]; imag[i] = imag[j]; imag[j] = ti;
        }
    }

    /* Cooley-Tukey decimation-in-time */
    for (len = 2; len <= n; len <<= 1) {
        double angle = 2.0 * PI / len * (is_inverse ? -1.0 : 1.0);
        double wlen_r = cos(angle);
        double wlen_i = sin(angle);
        int half = len >> 1;

        for (i = 0; i < n; i += len) {
            double w_r = 1.0;
            double w_i = 0.0;
            for (j = 0; j < half; j++) {
                int u_idx = i + j;
                int v_idx = i + j + half;
                double u_r = real[u_idx];
                double u_i = imag[u_idx];
                double v_r = real[v_idx] * w_r - imag[v_idx] * w_i;
                double v_i = real[v_idx] * w_i + imag[v_idx] * w_r;

                real[u_idx] = u_r + v_r;
                imag[u_idx] = u_i + v_i;
                real[v_idx] = u_r - v_r;
                imag[v_idx] = u_i - v_i;

                double next_w_r = w_r * wlen_r - w_i * wlen_i;
                double next_w_i = w_r * wlen_i + w_i * wlen_r;
                w_r = next_w_r;
                w_i = next_w_i;
            }
        }
    }

    if (is_inverse) {
        for (i = 0; i < n; i++) {
            real[i] /= n;
            imag[i] /= n;
        }
    }
}

#define N (1 << 16) // 65536 points

int main(void) {
    double *real = (double *)malloc(N * sizeof(double));
    double *imag = (double *)malloc(N * sizeof(double));
    if (!real || !imag) return 1;

    /* Initialize with multitone test signal */
    for (int i = 0; i < N; i++) {
        real[i] = sin(2.0 * PI * i * 50.0 / N) + 0.5 * cos(2.0 * PI * i * 120.0 / N) + 0.2 * sin(2.0 * PI * i * 300.0 / N);
        imag[i] = 0.0;
    }

    /* Repeat FFT + IFFT multiple rounds */
    int rounds = 16;
    for (int r = 0; r < rounds; r++) {
        fft(real, imag, N, 0); // Forward FFT
        /* Spectral modification */
        for (int i = 0; i < N; i++) {
            real[i] *= 1.0001;
            imag[i] *= 1.0001;
        }
        fft(real, imag, N, 1); // Inverse FFT
    }

    double power_sum = 0.0;
    for (int i = 0; i < N; i++) {
        power_sum += real[i] * real[i] + imag[i] * imag[i];
    }

    printf("FFT: N=%d Rounds=%d PowerSum=%.4f\n", N, rounds, power_sum);

    free(real);
    free(imag);
    return 0;
}
