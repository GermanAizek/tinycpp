#include <stdio.h>
#include <stdlib.h>

#define N 400

static double A[N][N];
static double B[N][N];
static double C[N][N];

int main(void) {
    int i, j, k;
    double sum = 0.0;

    /* Initialize matrices */
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            A[i][j] = (double)((i * 3 + j * 7) % 100) / 10.0;
            B[i][j] = (double)((i * 5 + j * 2) % 100) / 10.0;
            C[i][j] = 0.0;
        }
    }

    /* Matrix multiplication */
    for (i = 0; i < N; i++) {
        for (k = 0; k < N; k++) {
            double r = A[i][k];
            for (j = 0; j < N; j++) {
                C[i][j] += r * B[k][j];
            }
        }
    }

    /* Compute checksum */
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            sum += C[i][j];
        }
    }

    printf("MatrixMul: N=%d Checksum=%.2f\n", N, sum);
    return 0;
}
