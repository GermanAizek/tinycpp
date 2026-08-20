#include <stdio.h>

#define N 13

class NQueensContext {
public:
    int solutions;
    int col_mask;
    int diag1_mask;
    int diag2_mask;
};

void nqueens_solve(NQueensContext &ctx, int row) {
    if (row == N) {
        ctx.solutions++;
        return;
    }
    for (int col = 0; col < N; col++) {
        int d1 = row + col;
        int d2 = row - col + N;
        if (!(ctx.col_mask & (1 << col)) &&
            !(ctx.diag1_mask & (1 << d1)) &&
            !(ctx.diag2_mask & (1 << d2))) {
            ctx.col_mask ^= (1 << col);
            ctx.diag1_mask ^= (1 << d1);
            ctx.diag2_mask ^= (1 << d2);

            nqueens_solve(ctx, row + 1);

            ctx.col_mask ^= (1 << col);
            ctx.diag1_mask ^= (1 << d1);
            ctx.diag2_mask ^= (1 << d2);
        }
    }
}

int main() {
    NQueensContext ctx;
    ctx.solutions = 0;
    ctx.col_mask = 0;
    ctx.diag1_mask = 0;
    ctx.diag2_mask = 0;

    nqueens_solve(ctx, 0);

    printf("CppNQueens: N=%d Solutions=%d\n", N, ctx.solutions);
    return 0;
}
