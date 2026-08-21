#include <stdio.h>

#define N 13

static int solutions = 0;
static int col_mask = 0;
static int diag1_mask = 0;
static int diag2_mask = 0;

static void solve(int row) {
    if (row == N) {
        solutions++;
        return;
    }
    for (int col = 0; col < N; col++) {
        int d1 = row + col;
        int d2 = row - col + N;
        if (!(col_mask & (1 << col)) &&
            !(diag1_mask & (1 << d1)) &&
            !(diag2_mask & (1 << d2))) {
            col_mask ^= (1 << col);
            diag1_mask ^= (1 << d1);
            diag2_mask ^= (1 << d2);

            solve(row + 1);

            col_mask ^= (1 << col);
            diag1_mask ^= (1 << d1);
            diag2_mask ^= (1 << d2);
        }
    }
}

int main(void) {
    solve(0);
    printf("NQueens: N=%d Solutions=%d\n", N, solutions);
    return 0;
}
