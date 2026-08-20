#include <stdio.h>
#include <stdlib.h>

#define N 400

class Matrix {
public:
    double data[N][N];
};

void matrix_init(Matrix &m, int factor_a, int factor_b) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            m.data[i][j] = (double)((i * factor_a + j * factor_b) % 100) / 10.0;
        }
    }
}

void matrix_zero(Matrix &m) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            m.data[i][j] = 0.0;
        }
    }
}

void matrix_multiply(Matrix &c, const Matrix &a, const Matrix &b) {
    matrix_zero(c);
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < N; k++) {
            double r = a.data[i][k];
            for (int j = 0; j < N; j++) {
                c.data[i][j] += r * b.data[k][j];
            }
        }
    }
}

double matrix_checksum(const Matrix &m) {
    double sum = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            sum += m.data[i][j];
        }
    }
    return sum;
}

static Matrix mat_a;
static Matrix mat_b;
static Matrix mat_c;

int main() {
    matrix_init(mat_a, 3, 7);
    matrix_init(mat_b, 5, 2);
    matrix_multiply(mat_c, mat_a, mat_b);

    printf("CppMatrixMul: N=%d Checksum=%.2f\n", N, matrix_checksum(mat_c));
    return 0;
}
