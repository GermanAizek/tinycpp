#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846

struct ComplexVal {
    double r;
    double i;
};

namespace DSP {

static void compute_fft(ComplexVal *data, int n, bool is_inverse) {
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            ComplexVal tmp = data[i];
            data[i] = data[j];
            data[j] = tmp;
        }
    }

    for (int len = 2; len <= n; len <<= 1) {
        double angle = 2.0 * PI / static_cast<double>(len) * (is_inverse ? -1.0 : 1.0);
        double wlen_r = cos(angle);
        double wlen_i = sin(angle);
        int half = len >> 1;

        for (int i = 0; i < n; i += len) {
            double w_r = 1.0;
            double w_i = 0.0;
            for (int j = 0; j < half; j++) {
                int u_idx = i + j;
                int v_idx = i + j + half;
                double u_r = data[u_idx].r;
                double u_i = data[u_idx].i;
                double v_r = data[v_idx].r * w_r - data[v_idx].i * w_i;
                double v_i = data[v_idx].r * w_i + data[v_idx].i * w_r;

                data[u_idx].r = u_r + v_r;
                data[u_idx].i = u_i + v_i;
                data[v_idx].r = u_r - v_r;
                data[v_idx].i = u_i - v_i;

                double next_w_r = w_r * wlen_r - w_i * wlen_i;
                double next_w_i = w_r * wlen_i + w_i * wlen_r;
                w_r = next_w_r;
                w_i = next_w_i;
            }
        }
    }

    if (is_inverse) {
        double inv_n = 1.0 / static_cast<double>(n);
        for (int i = 0; i < n; i++) {
            data[i].r *= inv_n;
            data[i].i *= inv_n;
        }
    }
}

} // namespace DSP

#define N (1 << 16) // 65536 points

int main(void) {
    ComplexVal *signal = static_cast<ComplexVal*>(malloc(N * sizeof(ComplexVal)));
    if (signal == nullptr) return 1;

    for (int i = 0; i < N; i++) {
        double t = static_cast<double>(i);
        signal[i].r = sin(2.0 * PI * t * 50.0 / N) + 0.5 * cos(2.0 * PI * t * 120.0 / N) + 0.2 * sin(2.0 * PI * t * 300.0 / N);
        signal[i].i = 0.0;
    }

    int rounds = 16;
    for (int r = 0; r < rounds; r++) {
        DSP::compute_fft(signal, N, false);
        for (int i = 0; i < N; i++) {
            signal[i].r *= 1.0001;
            signal[i].i *= 1.0001;
        }
        DSP::compute_fft(signal, N, true);
    }

    double power_sum = 0.0;
    for (int i = 0; i < N; i++) {
        power_sum += signal[i].r * signal[i].r + signal[i].i * signal[i].i;
    }

    printf("CPP_FFT: N=%d Rounds=%d PowerSum=%.4f\n", N, rounds, power_sum);

    free(signal);
    return 0;
}
